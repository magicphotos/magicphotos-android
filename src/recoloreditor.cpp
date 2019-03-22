#include <QtCore/QtMath>
#include <QtCore/QFileInfo>
#include <QtGui/QColor>
#include <QtGui/QTransform>
#include <QtGui/QImageReader>
#include <QtGui/QPainter>

#include "recoloreditor.h"

RecolorEditor::RecolorEditor(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    IsChanged    = false;
    CurrentMode  = ModeScroll;
    BrushSize    = 0;
    HelperSize   = 0;
    CurrentHue   = 0;
    BrushOpacity = 0.0;

    QRgb   rgb;
    QColor color;

    for (int i = 0; i < 65536; i++) {
        quint8 r, g, b;

        std::tie(r, g, b) = UnpackRGB16(static_cast<quint16>(i));

        rgb = qRgb(r, g, b);

        color.setRgb(rgb);

        RGB16ToHSVMap[static_cast<quint16>(i)] = PackHSV(static_cast<qint16>(color.hue()),
                                                         static_cast<quint8>(color.saturation()),
                                                         static_cast<quint8>(color.value()));
    }

    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);

    setFlag(QQuickItem::ItemHasContents, true);

    QObject::connect(this, &RecolorEditor::scaleChanged, this, &RecolorEditor::scaleWasChanged);
}

bool RecolorEditor::changed() const
{
    return IsChanged;
}

int RecolorEditor::mode() const
{
    return CurrentMode;
}

void RecolorEditor::setMode(int mode)
{
    CurrentMode = mode;
}

int RecolorEditor::brushSize() const
{
    return BrushSize;
}

void RecolorEditor::setBrushSize(int size)
{
    BrushSize = size;

    BrushTemplateImage = QImage(BrushSize * 2, BrushSize * 2, QImage::Format_ARGB32);

    for (int y = 0; y < BrushTemplateImage.height(); y++) {
        for (int x = 0; x < BrushTemplateImage.width(); x++) {
            qreal r = qSqrt(qPow(x - BrushSize, 2) + qPow(y - BrushSize, 2));

            if (r <= BrushSize) {
                if (r <= BrushSize * BrushOpacity) {
                    BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, 0xFF));
                } else {
                    BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, qFloor(0xFF * (BrushSize - r) / (BrushSize * (1.0 - BrushOpacity)))));
                }
            } else {
                BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, 0x00));
            }
        }
    }

    int brush_width = qMax(1, qMin(qMin(qFloor(BrushSize / scale()) * 2, CurrentImage.width()), CurrentImage.height()));

    BrushImage = BrushTemplateImage.scaledToWidth(brush_width);
}

int RecolorEditor::helperSize() const
{
    return HelperSize;
}

void RecolorEditor::setHelperSize(int size)
{
    HelperSize = size;
}

int RecolorEditor::hue() const
{
    return CurrentHue;
}

void RecolorEditor::setHue(int hue)
{
    CurrentHue = hue;
}

qreal RecolorEditor::brushOpacity() const
{
    return BrushOpacity;
}

void RecolorEditor::setBrushOpacity(qreal opacity)
{
    BrushOpacity = opacity;

    BrushTemplateImage = QImage(BrushSize * 2, BrushSize * 2, QImage::Format_ARGB32);

    for (int y = 0; y < BrushTemplateImage.height(); y++) {
        for (int x = 0; x < BrushTemplateImage.width(); x++) {
            qreal r = qSqrt(qPow(x - BrushSize, 2) + qPow(y - BrushSize, 2));

            if (r <= BrushSize) {
                if (r <= BrushSize * BrushOpacity) {
                    BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, 0xFF));
                } else {
                    BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, qFloor(0xFF * (BrushSize - r) / (BrushSize * (1.0 - BrushOpacity)))));
                }
            } else {
                BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, 0x00));
            }
        }
    }

    int brush_width = qMax(1, qMin(qMin(qFloor(BrushSize / scale()) * 2, CurrentImage.width()), CurrentImage.height()));

    BrushImage = BrushTemplateImage.scaledToWidth(brush_width);
}

void RecolorEditor::openImage(const QString &image_file, int image_orientation)
{
    if (!image_file.isNull()) {
        QImageReader reader(image_file);

        if (reader.canRead()) {
            QSize size = reader.size();

            if (size.width() * size.height() > IMAGE_MPIX_LIMIT * 1000000.0) {
                qreal factor = qSqrt((size.width() * size.height()) / (IMAGE_MPIX_LIMIT * 1000000.0));

                size.setWidth(qFloor(size.width()   / factor));
                size.setHeight(qFloor(size.height() / factor));

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
                    OriginalImage = LoadedImage;
                    CurrentImage  = LoadedImage;

                    LoadedImage = QImage();

                    UndoStack.clear();

                    IsChanged = false;

                    setImplicitWidth(CurrentImage.width());
                    setImplicitHeight(CurrentImage.height());

                    update();

                    int brush_width = qMax(1, qMin(qMin(qFloor(BrushSize / scale()) * 2, CurrentImage.width()), CurrentImage.height()));

                    BrushImage = BrushTemplateImage.scaledToWidth(brush_width);

                    emit undoAvailabilityChanged(false);
                    emit imageOpened();
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

void RecolorEditor::saveImage(const QString &image_file)
{
    QString file_name = image_file;

    if (!file_name.isNull()) {
        if (!CurrentImage.isNull()) {
            if (QFileInfo(file_name).suffix().compare("png", Qt::CaseInsensitive) != 0 &&
                QFileInfo(file_name).suffix().compare("jpg", Qt::CaseInsensitive) != 0 &&
                QFileInfo(file_name).suffix().compare("bmp", Qt::CaseInsensitive) != 0) {
                file_name = file_name + ".jpg";
            }

            if (CurrentImage.convertToFormat(QImage::Format_ARGB32).save(file_name)) {
                IsChanged = false;

                emit imageSaved(file_name);
            } else {
                emit imageSaveFailed();
            }
        } else {
            emit imageSaveFailed();
        }
    } else {
        emit imageSaveFailed();
    }
}

void RecolorEditor::undo()
{
    if (UndoStack.count() > 0) {
        CurrentImage = UndoStack.pop();

        if (UndoStack.count() == 0) {
            emit undoAvailabilityChanged(false);
        }

        IsChanged = true;

        update();
    }
}

void RecolorEditor::paint(QPainter *painter)
{
    painter->drawImage(QRectF(0, 0, width(), height()), CurrentImage, QRectF(0, 0, CurrentImage.width(), CurrentImage.height()));
}

void RecolorEditor::scaleWasChanged()
{
    int brush_width = qMax(1, qMin(qMin(qFloor(BrushSize / scale()) * 2, CurrentImage.width()), CurrentImage.height()));

    BrushImage = BrushTemplateImage.scaledToWidth(brush_width);
}

void RecolorEditor::mousePressEvent(QMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        ChangeImageAt(true, event->pos().x(), event->pos().y());

        emit mouseEvent(MousePressed, event->pos().x(), event->pos().y());
    }
}

void RecolorEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        ChangeImageAt(false, event->pos().x(), event->pos().y());

        emit mouseEvent(MouseMoved, event->pos().x(), event->pos().y());
    }
}

void RecolorEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        emit mouseEvent(MouseReleased, event->pos().x(), event->pos().y());
    }
}

quint16 RecolorEditor::PackRGB16(quint8 r, quint8 g, quint8 b)
{
    return static_cast<quint16>(((static_cast<quint16>(r) & 0xf8) << 8) |
                                ((static_cast<quint16>(g) & 0xfc) << 3) |
                                ((static_cast<quint16>(b) & 0xf8) >> 3));
}

std::tuple<quint8, quint8, quint8> RecolorEditor::UnpackRGB16(quint16 rgb)
{
    auto r = static_cast<quint8>(((rgb >> 11) & 0x1f) << 3);
    auto g = static_cast<quint8>(((rgb >> 5)  & 0x3f) << 2);
    auto b = static_cast<quint8>(((rgb)       & 0x1f) << 3);

    return std::make_tuple(r, g, b);
}

quint32 RecolorEditor::PackHSV(qint16 h, quint8 s, quint8 v)
{
    quint16 n_h = h % 360 >= 0 ? h % 360 : h % 360 + 360;

    return static_cast<quint32>((static_cast<quint32>(n_h) << 16) |
                                (static_cast<quint32>(s)   << 8)  |
                                 static_cast<quint32>(v));
}

std::tuple<qint16, quint8, quint8> RecolorEditor::UnpackHSV(quint32 hsv)
{
    auto h = static_cast<qint16>((hsv >> 16) & 0xffff);
    auto s = static_cast<quint8>((hsv >> 8)  & 0xff);
    auto v = static_cast<quint8>((hsv)       & 0xff);

    return std::make_tuple(h, s, v);
}

QRgb RecolorEditor::AdjustHue(QRgb rgb)
{
    quint8 s, v;

    std::tie(std::ignore, s, v) = UnpackHSV(RGB16ToHSVMap[PackRGB16(static_cast<quint8>(qRed(rgb)),
                                                                    static_cast<quint8>(qGreen(rgb)),
                                                                    static_cast<quint8>(qBlue(rgb)))]);

    return QColor::fromHsv(CurrentHue, s, v, qAlpha(rgb)).rgba();
}

void RecolorEditor::SaveUndoImage()
{
    UndoStack.push(CurrentImage);

    if (UndoStack.count() > UNDO_DEPTH) {
        for (int i = 0; i < UndoStack.count() - UNDO_DEPTH; i++) {
            UndoStack.remove(0);
        }
    }

    emit undoAvailabilityChanged(true);
}

void RecolorEditor::ChangeImageAt(bool save_undo, int center_x, int center_y)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        if (save_undo) {
            SaveUndoImage();
        }

        int width  = qMin(BrushImage.width(),  CurrentImage.width());
        int height = qMin(BrushImage.height(), CurrentImage.height());

        int img_x = qMin(qMax(0, center_x - width  / 2), CurrentImage.width()  - width);
        int img_y = qMin(qMax(0, center_y - height / 2), CurrentImage.height() - height);

        QImage   brush_image(width, height, QImage::Format_ARGB32);
        QPainter brush_painter(&brush_image);

        if (CurrentMode == ModeOriginal) {
            brush_painter.setCompositionMode(QPainter::CompositionMode_Source);
            brush_painter.drawImage(QPoint(0, 0), OriginalImage, QRect(img_x, img_y, width, height));
        } else if (CurrentMode == ModeEffected) {
            for (int y = img_y; y < img_y + height; y++) {
                for (int x = img_x; x < img_x + width; x++) {
                    brush_image.setPixel(x - img_x, y - img_y, AdjustHue(OriginalImage.pixel(x, y)));
                }
            }
        }

        QPainter image_painter(&CurrentImage);

        brush_painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        brush_painter.drawImage(QPoint(0, 0), BrushImage);

        image_painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        image_painter.drawImage(QPoint(img_x, img_y), brush_image);

        IsChanged = true;

        update();

        QImage helper_image = CurrentImage.copy(center_x - qFloor((HelperSize / scale()) / 2),
                                                center_y - qFloor((HelperSize / scale()) / 2),
                                                qFloor(HelperSize / scale()),
                                                qFloor(HelperSize / scale())).scaledToWidth(HelperSize);

        emit helperImageReady(helper_image);
    }
}
