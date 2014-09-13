#include <QtCore/qmath.h>
#include <QtCore/QFileInfo>
#include <QtGui/QColor>
#include <QtGui/QTransform>
#include <QtGui/QImageReader>
#include <QtGui/QPainter>

#include "recoloreditor.h"

RecolorEditor::RecolorEditor(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    IsChanged   = false;
    CurrentMode = ModeScroll;
    HelperSize  = 0;
    ScreenDPI   = 0;
    CurrentHue  = 0;

    RGB16  rgb16;
    HSV    hsv;
    QRgb   rgb;
    QColor color;

    for (int i = 0; i < 65536; i++) {
        rgb16.rgb = i;

        rgb = qRgb(rgb16.r << 3, rgb16.g << 2, rgb16.b << 3);

        color.setRgb(rgb);

        hsv.h = color.hue();
        hsv.s = color.saturation();
        hsv.v = color.value();

        RGB16ToHSVMap[rgb16.rgb] = hsv.hsv;
    }

    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);

    setRenderTarget(QQuickPaintedItem::FramebufferObject);

    setFlag(QQuickItem::ItemHasContents, true);
}

RecolorEditor::~RecolorEditor()
{
}

int RecolorEditor::mode() const
{
    return CurrentMode;
}

void RecolorEditor::setMode(const int &mode)
{
    CurrentMode = mode;
}

int RecolorEditor::helperSize() const
{
    return HelperSize;
}

void RecolorEditor::setHelperSize(const int &size)
{
    HelperSize = size;
}

int RecolorEditor::screenDPI() const
{
    return ScreenDPI;
}

void RecolorEditor::setScreenDPI(const int &dpi)
{
    ScreenDPI = dpi;
}

int RecolorEditor::hue() const
{
    return CurrentHue;
}

void RecolorEditor::setHue(const int &hue)
{
    CurrentHue = hue;
}

bool RecolorEditor::changed() const
{
    return IsChanged;
}

void RecolorEditor::openImage(const QString &image_file, const int &image_orientation)
{
    if (!image_file.isNull()) {
        QImageReader reader(image_file);

        if (reader.canRead()) {
            QSize size = reader.size();

            if (size.width() * size.height() > IMAGE_MPIX_LIMIT * 1000000.0) {
                qreal factor = qSqrt((size.width() * size.height()) / (IMAGE_MPIX_LIMIT * 1000000.0));

                size.setWidth(size.width()   / factor);
                size.setHeight(size.height() / factor);

                reader.setScaledSize(size);
            }

            LoadedImage = reader.read();

            if (!LoadedImage.isNull()) {
                if (image_orientation == 3) {
                    QTransform transform;

                    transform.rotate(180);

                    LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.width(), LoadedImage.height());
                } else if (image_orientation == 6) {
                    QTransform transform;

                    transform.rotate(90);

                    LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
                } else if (image_orientation == 8) {
                    QTransform transform;

                    transform.rotate(270);

                    LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
                }

                LoadedImage = LoadedImage.convertToFormat(QImage::Format_RGB16);

                if (!LoadedImage.isNull()) {
                    OriginalImage = LoadedImage;
                    CurrentImage  = LoadedImage;

                    LoadedImage = QImage();

                    UndoStack.clear();

                    IsChanged = false;

                    setImplicitWidth(CurrentImage.width());
                    setImplicitHeight(CurrentImage.height());

                    update();

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
    if (UndoStack.size() > 0) {
        CurrentImage = UndoStack.pop();

        if (UndoStack.size() == 0) {
            emit undoAvailabilityChanged(false);
        }

        IsChanged = true;

        update();
    }
}

void RecolorEditor::paint(QPainter *painter)
{
    bool smooth_pixmap = painter->testRenderHint(QPainter::SmoothPixmapTransform);

    if (smooth()) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    } else {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
    }

    painter->drawPixmap(contentsBoundingRect(), QPixmap::fromImage(CurrentImage), QRectF(0, 0, CurrentImage.width(), CurrentImage.height()));

    painter->setRenderHint(QPainter::SmoothPixmapTransform, smooth_pixmap);
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

QRgb RecolorEditor::AdjustHue(QRgb rgb)
{
    RGB16 rgb16;
    HSV   hsv;

    rgb16.r = (qRed(rgb)   & 0xf8) >> 3;
    rgb16.g = (qGreen(rgb) & 0xfc) >> 2;
    rgb16.b = (qBlue(rgb)  & 0xf8) >> 3;

    hsv.hsv = RGB16ToHSVMap[rgb16.rgb];

    return QColor::fromHsv(CurrentHue, hsv.s, hsv.v, qAlpha(rgb)).rgba();
}

int RecolorEditor::MapSizeToDevice(int size)
{
    if (ScreenDPI >= 640) {
        return size * 4;
    } else if (ScreenDPI >= 480) {
        return size * 3;
    } else if (ScreenDPI >= 320) {
        return size * 2;
    } else if (ScreenDPI >= 240) {
        return size * 1.5;
    } else {
        return size;
    }
}

void RecolorEditor::SaveUndoImage()
{
    UndoStack.push(CurrentImage);

    if (UndoStack.size() > UNDO_DEPTH) {
        for (int i = 0; i < UndoStack.size() - UNDO_DEPTH; i++) {
            UndoStack.remove(0);
        }
    }

    emit undoAvailabilityChanged(true);
}

void RecolorEditor::ChangeImageAt(bool save_undo, int center_x, int center_y)
{
    if (CurrentMode != ModeScroll) {
        if (save_undo) {
            SaveUndoImage();
        }

        int radius = MapSizeToDevice(BRUSH_SIZE) / scale();

        for (int x = center_x - radius; x <= center_x + radius; x++) {
            for (int y = center_y - radius; y <= center_y + radius; y++) {
                if (x >= 0 && x < CurrentImage.width() && y >= 0 && y < CurrentImage.height() && qSqrt(qPow(x - center_x, 2) + qPow(y - center_y, 2)) <= radius) {
                    if (CurrentMode == ModeOriginal) {
                        CurrentImage.setPixel(x, y, OriginalImage.pixel(x, y));
                    } else {
                        CurrentImage.setPixel(x, y, AdjustHue(OriginalImage.pixel(x, y)));
                    }
                }
            }
        }

        IsChanged = true;

        update();

        QImage helper_image = CurrentImage.copy(center_x - (HelperSize / scale()) / 2,
                                                center_y - (HelperSize / scale()) / 2,
                                                HelperSize / scale(),
                                                HelperSize / scale()).scaledToWidth(HelperSize);

        emit helperImageReady(helper_image);
    }
}
