#include <qmath.h>
#include <QFileInfo>
#include <QColor>
#include <QImageReader>
#include <QPainter>

#include "recoloreditor.h"

RecolorEditor::RecolorEditor(QDeclarativeItem *parent) : QDeclarativeItem(parent)
{
    IsChanged   = false;
    CurrentMode = ModeScroll;
    HelperSize  = 0;
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

    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);
    setFlag(QGraphicsItem::ItemHasNoContents,           false);
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

void RecolorEditor::openImage(const QString &image_url)
{
    QString image_file = QUrl(image_url).toLocalFile();

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

void RecolorEditor::saveImage(const QString &image_url)
{
    QString file_name = QUrl(image_url).toLocalFile();

    if (!file_name.isNull()) {
        if (!CurrentImage.isNull()) {
            if (QFileInfo(file_name).suffix().compare("png", Qt::CaseInsensitive) != 0 &&
                QFileInfo(file_name).suffix().compare("jpg", Qt::CaseInsensitive) != 0 &&
                QFileInfo(file_name).suffix().compare("bmp", Qt::CaseInsensitive) != 0) {
                file_name = file_name + ".jpg";
            }

            if (CurrentImage.convertToFormat(QImage::Format_ARGB32).save(file_name)) {
                IsChanged = false;

                emit imageSaved();
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

void RecolorEditor::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget*)
{
    qreal scale = 1.0;

    if (CurrentImage.width() != 0 && CurrentImage.height() != 0) {
        scale = width() / CurrentImage.width() < height() / CurrentImage.height() ?
                width() / CurrentImage.width() : height() / CurrentImage.height();
    }

    bool antialiasing = painter->testRenderHint(QPainter::Antialiasing);

    if (smooth()) {
        painter->setRenderHint(QPainter::Antialiasing, true);
    }

    QRectF src_rect(option->exposedRect.left()   / scale,
                    option->exposedRect.top()    / scale,
                    option->exposedRect.width()  / scale,
                    option->exposedRect.height() / scale);

    painter->drawImage(option->exposedRect, CurrentImage.copy(src_rect.toRect()));

    painter->setRenderHint(QPainter::Antialiasing, antialiasing);
}

void RecolorEditor::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        ChangeImageAt(true, event->pos().x(), event->pos().y());

        emit mouseEvent(MousePressed, event->pos().x(), event->pos().y());
    }
}

void RecolorEditor::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        ChangeImageAt(false, event->pos().x(), event->pos().y());

        emit mouseEvent(MouseMoved, event->pos().x(), event->pos().y());
    }
}

void RecolorEditor::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
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

        qreal scale = 1.0;

        if (CurrentImage.width() != 0 && CurrentImage.height() != 0) {
            scale = width() / CurrentImage.width() < height() / CurrentImage.height() ?
                    width() / CurrentImage.width() : height() / CurrentImage.height();
        }

        int img_center_x = center_x   / scale;
        int img_center_y = center_y   / scale;
        int radius       = BRUSH_SIZE / scale;

        for (int x = img_center_x - radius; x <= img_center_x + radius; x++) {
            for (int y = img_center_y - radius; y <= img_center_y + radius; y++) {
                if (x >= 0 && x < CurrentImage.width() && y >= 0 && y < CurrentImage.height() && qSqrt(qPow(x - img_center_x, 2) + qPow(y - img_center_y, 2)) <= radius) {
                    if (CurrentMode == ModeOriginal) {
                        CurrentImage.setPixel(x, y, OriginalImage.pixel(x, y));
                    } else {
                        CurrentImage.setPixel(x, y, AdjustHue(OriginalImage.pixel(x, y)));
                    }
                }
            }
        }

        IsChanged = true;

        update(center_x - BRUSH_SIZE, center_y - BRUSH_SIZE, BRUSH_SIZE * 2, BRUSH_SIZE * 2);

        QImage helper_image = CurrentImage.copy(img_center_x - (HelperSize / scale) / 2,
                                                img_center_y - (HelperSize / scale) / 2,
                                                HelperSize / scale,
                                                HelperSize / scale).scaledToWidth(HelperSize);

        emit helperImageReady(helper_image);
    }
}
