#include <QtCore/QtMath>
#include <QtCore/QSize>
#include <QtCore/QRectF>
#include <QtCore/QFileInfo>
#include <QtGui/QColor>
#include <QtGui/QTransform>
#include <QtGui/QImageReader>

#include "editor.h"

Editor::Editor(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    Changed       = false;
    Mode          = ModeScroll;
    BrushSize     = 0;
    HelperSize    = 0;
    BrushHardness = 0.0;

    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);

    setFlag(QQuickItem::ItemHasContents, true);

    connect(this, &Editor::scaleChanged, this, &Editor::scaleBrushImage);
}

bool Editor::changed() const
{
    return Changed;
}

int Editor::mode() const
{
    return Mode;
}

void Editor::setMode(int mode)
{
    Mode = mode;
}

int Editor::brushSize() const
{
    return BrushSize;
}

void Editor::setBrushSize(int size)
{
    BrushSize = size;

    if (BrushSize > 0) {
        BrushTemplateImage = QImage(BrushSize * 2, BrushSize * 2, QImage::Format_ARGB32);

        for (int y = 0; y < BrushTemplateImage.height(); y++) {
            for (int x = 0; x < BrushTemplateImage.width(); x++) {
                qreal r = qSqrt(qPow(x - BrushSize, 2) + qPow(y - BrushSize, 2));

                if (r <= BrushSize) {
                    if (r <= BrushSize * BrushHardness) {
                        BrushTemplateImage.setPixel(x, y, qRgba(255, 255, 255, 255));
                    } else {
                        BrushTemplateImage.setPixel(x, y, qRgba(255, 255, 255, qFloor(255 * (BrushSize - r) / (BrushSize * (1.0 - BrushHardness)))));
                    }
                } else {
                    BrushTemplateImage.setPixel(x, y, qRgba(255, 255, 255, 0));
                }
            }
        }

        int brush_width = qMax(1, qMin(qMin(qFloor(BrushSize / scale()) * 2, CurrentImage.width()), CurrentImage.height()));

        if (brush_width > 0) {
            BrushImage = BrushTemplateImage.scaledToWidth(brush_width);
        } else {
            BrushImage = QImage();
        }
    } else {
        BrushTemplateImage = QImage();
        BrushImage         = QImage();
    }
}

int Editor::helperSize() const
{
    return HelperSize;
}

void Editor::setHelperSize(int size)
{
    HelperSize = size;
}

qreal Editor::brushHardness() const
{
    return BrushHardness;
}

void Editor::setBrushHardness(qreal hardness)
{
    BrushHardness = hardness;

    if (BrushSize > 0) {
        BrushTemplateImage = QImage(BrushSize * 2, BrushSize * 2, QImage::Format_ARGB32);

        for (int y = 0; y < BrushTemplateImage.height(); y++) {
            for (int x = 0; x < BrushTemplateImage.width(); x++) {
                qreal r = qSqrt(qPow(x - BrushSize, 2) + qPow(y - BrushSize, 2));

                if (r <= BrushSize) {
                    if (r <= BrushSize * BrushHardness) {
                        BrushTemplateImage.setPixel(x, y, qRgba(255, 255, 255, 255));
                    } else {
                        BrushTemplateImage.setPixel(x, y, qRgba(255, 255, 255, qFloor(255 * (BrushSize - r) / (BrushSize * (1.0 - BrushHardness)))));
                    }
                } else {
                    BrushTemplateImage.setPixel(x, y, qRgba(255, 255, 255, 0));
                }
            }
        }

        int brush_width = qMax(1, qMin(qMin(qFloor(BrushSize / scale()) * 2, CurrentImage.width()), CurrentImage.height()));

        if (brush_width > 0) {
            BrushImage = BrushTemplateImage.scaledToWidth(brush_width);
        } else {
            BrushImage = QImage();
        }
    } else {
        BrushTemplateImage = QImage();
        BrushImage         = QImage();
    }
}

void Editor::openImage(const QString &image_path, int image_orientation)
{
    if (!image_path.isNull()) {
        QImageReader reader(image_path);

        if (reader.canRead()) {
            QSize size = reader.size();

            if (!size.isEmpty()) {
                if (static_cast<qreal>(size.width()) * static_cast<qreal>(size.height()) > IMAGE_MPIX_LIMIT * 1000000.0) {
                    qreal scale = qSqrt((static_cast<qreal>(size.width()) * static_cast<qreal>(size.height())) / (IMAGE_MPIX_LIMIT * 1000000.0));

                    size.setWidth(qFloor(size.width()   / scale));
                    size.setHeight(qFloor(size.height() / scale));
                }

                if (!size.isEmpty()) {
                    reader.setScaledSize(size);

                    LoadedImage = reader.read();

                    if (!LoadedImage.isNull()) {
                        if (image_orientation == 90) {
                            QTransform transform;

                            transform.rotate(90);

                            LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
                        } else if (image_orientation == 180) {
                            QTransform transform;

                            transform.rotate(180);

                            LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.width(), LoadedImage.height());
                        } else if (image_orientation == 270) {
                            QTransform transform;

                            transform.rotate(270);

                            LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
                        }

                        LoadedImage = LoadedImage.convertToFormat(QImage::Format_RGB32);

                        if (!LoadedImage.isNull()) {
                            UndoStack.clear();

                            emit undoAvailabilityUpdated(false);

                            processOpenedImage();
                        } else {
                            emit imageOpenFailed();
                        }
                    } else {
                        emit imageOpenFailed();
                    }
                } else {
                    emit imageOpenFailed();
                }
            } else {
                emit imageOpenFailed();
            }
        } else {
            emit imageOpenFailed();
        }
    } else {
        emit imageOpenFailed();
    }
}

void Editor::saveImage(const QString &image_path)
{
    QString file_path = image_path;

    if (!file_path.isNull() && !CurrentImage.isNull()) {
        if (QFileInfo(file_path).suffix().compare(QStringLiteral("png"), Qt::CaseInsensitive) != 0 &&
            QFileInfo(file_path).suffix().compare(QStringLiteral("jpg"), Qt::CaseInsensitive) != 0 &&
            QFileInfo(file_path).suffix().compare(QStringLiteral("bmp"), Qt::CaseInsensitive) != 0) {
            file_path = file_path + QStringLiteral(".jpg");
        }

        if (CurrentImage.convertToFormat(QImage::Format_ARGB32).save(file_path)) {
            Changed = false;

            emit imageSaved(file_path);
        } else {
            emit imageSaveFailed();
        }
    } else {
        emit imageSaveFailed();
    }
}

void Editor::undo()
{
    if (UndoStack.count() > 0) {
        CurrentImage = UndoStack.pop();

        if (UndoStack.count() == 0) {
            emit undoAvailabilityUpdated(false);
        }

        Changed = true;

        update();
    }
}

void Editor::paint(QPainter *painter)
{
    painter->drawImage(QRectF(0, 0, width(), height()), CurrentImage, QRectF(0, 0, CurrentImage.width(), CurrentImage.height()));
}

void Editor::scaleBrushImage()
{
    if (!BrushTemplateImage.isNull()) {
        int brush_width = qMax(1, qMin(qMin(qFloor(BrushSize / scale()) * 2, CurrentImage.width()), CurrentImage.height()));

        if (brush_width > 0) {
            BrushImage = BrushTemplateImage.scaledToWidth(brush_width);
        } else {
            BrushImage = QImage();
        }
    } else {
        BrushImage = QImage();
    }
}

void Editor::SaveUndoImage()
{
    UndoStack.push(CurrentImage);

    if (UndoStack.count() > UNDO_DEPTH) {
        for (int i = 0; i < UndoStack.count() - UNDO_DEPTH; i++) {
            UndoStack.remove(0);
        }
    }

    emit undoAvailabilityUpdated(true);
}
