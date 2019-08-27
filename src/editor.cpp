#include <QtCore/QtMath>
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

    QObject::connect(this, &Editor::scaleChanged, this, &Editor::scaleBrushImage);
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
                        BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, 0xFF));
                    } else {
                        BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, qFloor(0xFF * (BrushSize - r) / (BrushSize * (1.0 - BrushHardness)))));
                    }
                } else {
                    BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, 0x00));
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
                        BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, 0xFF));
                    } else {
                        BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, qFloor(0xFF * (BrushSize - r) / (BrushSize * (1.0 - BrushHardness)))));
                    }
                } else {
                    BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, 0x00));
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

void Editor::openImage(const QString &image_file, int image_orientation)
{
    if (!image_file.isNull()) {
        QImageReader reader(image_file);

        if (reader.canRead()) {
            QSize size = reader.size();

            if (size.width() * size.height() > IMAGE_MPIX_LIMIT * 1000000.0) {
                qreal scale = qSqrt((size.width() * size.height()) / (IMAGE_MPIX_LIMIT * 1000000.0));

                size.setWidth(qFloor(size.width()   / scale));
                size.setHeight(qFloor(size.height() / scale));

                reader.setScaledSize(size);
            }

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

                    emit undoAvailabilityChanged(false);

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
}

void Editor::saveImage(const QString &image_file)
{
    QString file_name = image_file;

    if (!file_name.isNull() && !CurrentImage.isNull()) {
        if (QFileInfo(file_name).suffix().compare("png", Qt::CaseInsensitive) != 0 &&
            QFileInfo(file_name).suffix().compare("jpg", Qt::CaseInsensitive) != 0 &&
            QFileInfo(file_name).suffix().compare("bmp", Qt::CaseInsensitive) != 0) {
            file_name = file_name + ".jpg";
        }

        if (CurrentImage.convertToFormat(QImage::Format_ARGB32).save(file_name)) {
            Changed = false;

            emit imageSaved(file_name);
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
            emit undoAvailabilityChanged(false);
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

    emit undoAvailabilityChanged(true);
}
