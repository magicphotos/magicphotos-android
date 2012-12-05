#include <QtCore/qmath.h>

#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QColor>
#include <QImageReader>
#include <QDesktopServices>
#include <QCoreApplication>

#include "paintitem.h"

PaintItem::PaintItem(QDeclarativeItem *parent) : QDeclarativeItem(parent)
{
    CurrentEffect = EFFECT_GRAYSCALE;
    CurrentMode   = MODE_SCROLL;
    ScalePercent  = 100;
    Hue           = 180;

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

PaintItem::~PaintItem()
{
}

QString PaintItem::currentImageUrl() const
{
    return CurrentImageURL;
}

void PaintItem::setEffect(int effect)
{
    QImage::Format format;

    if (effect >= 0 && effect < EffectedImages.size()) {
        EffectedImages.replace(CurrentEffect, EffectedImage);
        CurrentImages.replace(CurrentEffect, CurrentImage);

        CurrentEffect = effect;

        EffectedImage = EffectedImages.at(CurrentEffect);
        CurrentImage  = CurrentImages.at(CurrentEffect);

        if (!EffectedImage.isNull()) {
            format              = EffectedImage.format();
            ScaledEffectedImage = EffectedImage.scaledToWidth((EffectedImage.width() * ScalePercent) / 100).convertToFormat(format);
        } else {
            ScaledEffectedImage = QImage();
        }
        if (!CurrentImage.isNull()) {
            format              = CurrentImage.format();
            ScaledCurrentImage  = CurrentImage.scaledToWidth((CurrentImage.width() * ScalePercent) / 100).convertToFormat(format);
        } else {
            ScaledCurrentImage  = QImage();
        }

        UndoStack.clear();

        setImplicitWidth(ScaledCurrentImage.width());
        setImplicitHeight(ScaledCurrentImage.height());

        update();

        emit undoStackEmpty();
    }
}

void PaintItem::setMode(int mode)
{
    CurrentMode = mode;
}

void PaintItem::setScale(int scale)
{
    QImage::Format format;

    ScalePercent = scale;

    if (!OriginalImage.isNull()) {
        format              = OriginalImage.format();
        ScaledOriginalImage = OriginalImage.scaledToWidth((OriginalImage.width() * ScalePercent) / 100).convertToFormat(format);
    } else {
        ScaledOriginalImage = QImage();
    }
    if (!EffectedImage.isNull()) {
        format              = EffectedImage.format();
        ScaledEffectedImage = EffectedImage.scaledToWidth((EffectedImage.width() * ScalePercent) / 100).convertToFormat(format);
    } else {
        ScaledEffectedImage = QImage();
    }
    if (!CurrentImage.isNull()) {
        format              = CurrentImage.format();
        ScaledCurrentImage  = CurrentImage.scaledToWidth((CurrentImage.width() * ScalePercent) / 100).convertToFormat(format);
    } else {
        ScaledCurrentImage  = QImage();
    }

    setImplicitWidth(ScaledCurrentImage.width());
    setImplicitHeight(ScaledCurrentImage.height());

    update();
}

void PaintItem::setHue(int hue)
{
    if (hue < 0) {
        Hue = 0;
    } else if (hue > 359) {
        hue = 359;
    } else {
        Hue = hue;
    }
}

void PaintItem::openImage(const QString &image_url)
{
    QString file_name = QUrl(image_url).toLocalFile();

    if (!file_name.isNull()) {
        QImageReader reader(file_name);

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
                    CurrentImageURL = image_url;

                    PrepareImages();
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

void PaintItem::openImageFromResource(const QString &image_res)
{
    if (!image_res.isNull()) {
        LoadedImage = QImage(image_res).convertToFormat(QImage::Format_RGB16);

        if (!LoadedImage.isNull()) {
            QDir().mkpath(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));

            CurrentImageURL = QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation) + "/image.jpg").toString();

            PrepareImages();
        }
    }
}

void PaintItem::saveImage(const QString &image_url)
{
    if (!CurrentImage.isNull()) {
        QString file_name = QUrl(image_url).toLocalFile();

        if (!file_name.isNull()) {
            if (QFileInfo(file_name).suffix().compare("png", Qt::CaseInsensitive) != 0 &&
                QFileInfo(file_name).suffix().compare("jpg", Qt::CaseInsensitive) != 0 &&
                QFileInfo(file_name).suffix().compare("bmp", Qt::CaseInsensitive) != 0) {
                file_name = file_name + ".jpg";
            }

            if (CurrentImage.convertToFormat(QImage::Format_ARGB32).save(file_name)) {
                CurrentImageURL = image_url;

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

void PaintItem::undo()
{
    if (UndoStack.size() > 0) {
        QImage::Format format;

        CurrentImage = UndoStack.pop();

        if (!CurrentImage.isNull()) {
            format             = CurrentImage.format();
            ScaledCurrentImage = CurrentImage.scaledToWidth((CurrentImage.width() * ScalePercent) / 100).convertToFormat(format);
        } else {
            ScaledCurrentImage = QImage();
        }

        update();

        emit imageChanged();

        if (UndoStack.size() == 0) {
            emit undoStackEmpty();
        }
    }
}

void PaintItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget*)
{
    painter->drawImage(option->exposedRect, ScaledCurrentImage.copy(option->exposedRect.toRect()));
}

void PaintItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (CurrentMode == MODE_ORIGINAL || CurrentMode == MODE_EFFECTED) {
        SaveUndoImage();

        ChangeImageAt(event->pos().x(), event->pos().y());

        emit imageChanged();
    }
}

void PaintItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (CurrentMode == MODE_ORIGINAL || CurrentMode == MODE_EFFECTED) {
        ChangeImageAt(event->pos().x(), event->pos().y());
    }
}

void PaintItem::PrepareImages()
{
    QImage::Format format;

    EffectedImages.clear();
    CurrentImages.clear();

    MakeEffectedImages();

    for (int i = 0; i < EffectedImages.size(); i++) {
        if (i == EFFECT_HUE) {
            CurrentImages.append(LoadedImage);
        } else {
            CurrentImages.append(EffectedImages.at(i));
        }
    }

    OriginalImage = LoadedImage;
    EffectedImage = EffectedImages.at(CurrentEffect);
    CurrentImage  = CurrentImages.at(CurrentEffect);

    if (!OriginalImage.isNull()) {
        format              = OriginalImage.format();
        ScaledOriginalImage = OriginalImage.scaledToWidth((OriginalImage.width() * ScalePercent) / 100).convertToFormat(format);
    } else {
        ScaledOriginalImage = QImage();
    }
    if (!EffectedImage.isNull()) {
        format              = EffectedImage.format();
        ScaledEffectedImage = EffectedImage.scaledToWidth((EffectedImage.width() * ScalePercent) / 100).convertToFormat(format);
    } else {
        ScaledEffectedImage = QImage();
    }
    if (!CurrentImage.isNull()) {
        format              = CurrentImage.format();
        ScaledCurrentImage  = CurrentImage.scaledToWidth((CurrentImage.width() * ScalePercent) / 100).convertToFormat(format);
    } else {
        ScaledCurrentImage  = QImage();
    }

    LoadedImage = QImage();

    UndoStack.clear();

    setImplicitWidth(ScaledCurrentImage.width());
    setImplicitHeight(ScaledCurrentImage.height());

    update();

    emit imageOpened();
    emit imageChanged();
    emit undoStackEmpty();
}

void PaintItem::MakeEffectedImages()
{
    qint64 msec = QDateTime::currentMSecsSinceEpoch();

    QImage grayscale_image = LoadedImage;
    QImage sketch_image    = LoadedImage;
    QImage blurred_image   = LoadedImage;

    // Make Gaussian blur of original image

    QImage::Format format = blurred_image.format();

    blurred_image = blurred_image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
    int alpha = (GAUSSIAN_RADIUS < 1) ? 16 : (GAUSSIAN_RADIUS > 17) ? 1 : tab[GAUSSIAN_RADIUS - 1];

    int r1 = blurred_image.rect().top();
    int r2 = blurred_image.rect().bottom();
    int c1 = blurred_image.rect().left();
    int c2 = blurred_image.rect().right();

    int bpl = blurred_image.bytesPerLine();

    int           rgba[4];
    unsigned char *p;

    for (int col = c1; col <= c2; col++) {
        p = blurred_image.scanLine(r1) + col * 4;

        for (int i = 0; i < 4; i++) {
            rgba[i] = p[i] << 4;
        }

        p += bpl;

        for (int j = r1; j < r2; j++, p += bpl) {
            for (int i = 0; i < 4; i++) {
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
            }
        }

        if (QDateTime::currentMSecsSinceEpoch() - msec > EVENT_PROCESS_INTERVAL) {
            QCoreApplication::processEvents();

            msec = QDateTime::currentMSecsSinceEpoch();
        }
    }

    for (int row = r1; row <= r2; row++) {
        p = blurred_image.scanLine(row) + c1 * 4;

        for (int i = 0; i < 4; i++) {
            rgba[i] = p[i] << 4;
        }

        p += 4;

        for (int j = c1; j < c2; j++, p += 4) {
            for (int i = 0; i < 4; i++) {
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
            }
        }

        if (QDateTime::currentMSecsSinceEpoch() - msec > EVENT_PROCESS_INTERVAL) {
            QCoreApplication::processEvents();

            msec = QDateTime::currentMSecsSinceEpoch();
        }
    }

    for (int col = c1; col <= c2; col++) {

        p = blurred_image.scanLine(r2) + col * 4;

        for (int i = 0; i < 4; i++) {
            rgba[i] = p[i] << 4;
        }

        p -= bpl;

        for (int j = r1; j < r2; j++, p -= bpl) {
            for (int i = 0; i < 4; i++) {
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
            }
        }

        if (QDateTime::currentMSecsSinceEpoch() - msec > EVENT_PROCESS_INTERVAL) {
            QCoreApplication::processEvents();

            msec = QDateTime::currentMSecsSinceEpoch();
        }
    }

    for (int row = r1; row <= r2; row++) {
        p = blurred_image.scanLine(row) + c2 * 4;

        for (int i = 0; i < 4; i++) {
            rgba[i] = p[i] << 4;
        }

        p -= 4;

        for (int j = c1; j < c2; j++, p -= 4) {
            for (int i = 0; i < 4; i++) {
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
            }
        }

        if (QDateTime::currentMSecsSinceEpoch() - msec > EVENT_PROCESS_INTERVAL) {
            QCoreApplication::processEvents();

            msec = QDateTime::currentMSecsSinceEpoch();
        }
    }

    blurred_image = blurred_image.convertToFormat(format);

    // Make grayscale image from original image & inverted grayscale from blurred

    for (int x = 0; x < LoadedImage.width(); x++) {
        for (int y = 0; y < LoadedImage.height(); y++) {
            int gray  = qGray(LoadedImage.pixel(x, y));
            int alpha = qAlpha(LoadedImage.pixel(x, y));

            grayscale_image.setPixel(x, y, qRgba(gray, gray, gray, alpha));

            int blr_gray = qGray(blurred_image.pixel(x, y));
            int inv_gray = blr_gray >= 255 ? 0 : 255 - blr_gray;

            sketch_image.setPixel(x, y, qRgba(inv_gray, inv_gray, inv_gray, alpha));
        }

        if (QDateTime::currentMSecsSinceEpoch() - msec > EVENT_PROCESS_INTERVAL) {
            QCoreApplication::processEvents();

            msec = QDateTime::currentMSecsSinceEpoch();
        }
    }

    // Apply Color Dodge mixing to grayscale & inverted blurred grayscale images for sketch

    for (int x = 0; x < sketch_image.width(); x++) {
        for (int y = 0; y < sketch_image.height(); y++) {
            int top_gray = qGray(sketch_image.pixel(x, y));
            int btm_gray = qGray(grayscale_image.pixel(x, y));
            int res_gray = top_gray >= 255 ? 255 : qMin(btm_gray * 255 / (255 - top_gray), 255);

            int alpha    = qAlpha(sketch_image.pixel(x, y));

            sketch_image.setPixel(x, y, qRgba(res_gray, res_gray, res_gray, alpha));
        }

        if (QDateTime::currentMSecsSinceEpoch() - msec > EVENT_PROCESS_INTERVAL) {
            QCoreApplication::processEvents();

            msec = QDateTime::currentMSecsSinceEpoch();
        }
    }

    EffectedImages.append(grayscale_image); // Grayscale
    EffectedImages.append(sketch_image);    // Sketch
    EffectedImages.append(blurred_image);   // Blurred
    EffectedImages.append(QImage());        // Hue effect (no effected image)
}

void PaintItem::SaveUndoImage()
{
    UndoStack.push(CurrentImage);

    if (UndoStack.size() > UNDO_DEPTH) {
        for (int i = 0; i < UndoStack.size() - UNDO_DEPTH; i++) {
            UndoStack.remove(0);
        }
    }

    emit undoStackNotEmpty();
}

void PaintItem::ChangeImageAt(int center_x, int center_y)
{
    int radius = BRUSH_SIZE;

    for (int x = center_x - radius; x <= center_x + radius; x++) {
        for (int y = center_y - radius; y <= center_y + radius; y++) {
            if (x >= 0 && x < ScaledCurrentImage.width() && y >= 0 && y < ScaledCurrentImage.height() && qSqrt(qPow(x - center_x, 2) + qPow(y - center_y, 2)) <= radius) {
                if (CurrentMode == MODE_ORIGINAL) {
                    ScaledCurrentImage.setPixel(x, y, ScaledOriginalImage.pixel(x, y));
                } else {
                    if (CurrentEffect == EFFECT_HUE) {
                        ScaledCurrentImage.setPixel(x, y, AdjustHue(ScaledOriginalImage.pixel(x, y)));
                    } else {
                        ScaledCurrentImage.setPixel(x, y, ScaledEffectedImage.pixel(x, y));
                    }
                }
            }
        }
    }

    update(center_x - radius, center_y - radius, radius * 2, radius * 2);

    center_x = (center_x   * 100) / ScalePercent;
    center_y = (center_y   * 100) / ScalePercent;
    radius   = (BRUSH_SIZE * 100) / ScalePercent;

    for (int x = center_x - radius; x <= center_x + radius; x++) {
        for (int y = center_y - radius; y <= center_y + radius; y++) {
            if (x >= 0 && x < CurrentImage.width() && y >= 0 && y < CurrentImage.height() && qSqrt(qPow(x - center_x, 2) + qPow(y - center_y, 2)) <= radius) {
                if (CurrentMode == MODE_ORIGINAL) {
                    CurrentImage.setPixel(x, y, OriginalImage.pixel(x, y));
                } else {
                    if (CurrentEffect == EFFECT_HUE) {
                        CurrentImage.setPixel(x, y, AdjustHue(OriginalImage.pixel(x, y)));
                    } else {
                        CurrentImage.setPixel(x, y, EffectedImage.pixel(x, y));
                    }
                }
            }
        }
    }
}

QRgb PaintItem::AdjustHue(QRgb rgb)
{
    RGB16 rgb16;
    HSV   hsv;

    rgb16.r = (qRed(rgb)   & 0xf8) >> 3;
    rgb16.g = (qGreen(rgb) & 0xfc) >> 2;
    rgb16.b = (qBlue(rgb)  & 0xf8) >> 3;

    hsv.hsv = RGB16ToHSVMap[rgb16.rgb];

    return QColor::fromHsv(Hue, hsv.s, hsv.v, qAlpha(rgb)).rgba();
}
