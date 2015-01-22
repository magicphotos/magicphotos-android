#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtGui/QTransform>
#include <QtGui/QImageReader>
#include <QtGui/QPainter>

#include <libexif/exif-loader.h>
#include <libexif/exif-data.h>
#include <libexif/exif-entry.h>

#include "retoucheditor.h"

RetouchEditor::RetouchEditor() : bb::cascades::CustomControl()
{
    IsChanged            = false;
    IsSamplingPointValid = false;
    IsLastBlurPointValid = false;
    CurrentMode          = ModeScroll;
    BrushSize            = 0;
    HelperSize           = 0;
}

RetouchEditor::~RetouchEditor()
{
}

int RetouchEditor::mode() const
{
    return CurrentMode;
}

void RetouchEditor::setMode(const int &mode)
{
    CurrentMode = mode;
}

int RetouchEditor::brushSize() const
{
    return BrushSize;
}

void RetouchEditor::setBrushSize(const int &size)
{
    BrushSize = size;
}

int RetouchEditor::helperSize() const
{
    return HelperSize;
}

void RetouchEditor::setHelperSize(const int &size)
{
    HelperSize = size;
}

bool RetouchEditor::samplingPointValid() const
{
    return IsSamplingPointValid;
}

void RetouchEditor::setSamplingPointValid(const bool &valid)
{
    IsSamplingPointValid = valid;
}

bool RetouchEditor::lastBlurPointValid() const
{
    return IsLastBlurPointValid;
}

void RetouchEditor::setLastBlurPointValid(const bool &valid)
{
    IsLastBlurPointValid = valid;
}

int RetouchEditor::samplingPointX() const
{
    return SamplingPoint.x();
}

void RetouchEditor::setSamplingPointX(const int &x)
{
    SamplingPoint.setX(x);
}

int RetouchEditor::samplingPointY() const
{
    return SamplingPoint.y();
}

void RetouchEditor::setSamplingPointY(const int &y)
{
    SamplingPoint.setY(y);
}

int RetouchEditor::lastBlurPointX() const
{
    return LastBlurPoint.x();
}

void RetouchEditor::setLastBlurPointX(const int &x)
{
    LastBlurPoint.setX(x);
}

int RetouchEditor::lastBlurPointY() const
{
    return LastBlurPoint.y();
}

void RetouchEditor::setLastBlurPointY(const int &y)
{
    LastBlurPoint.setY(y);
}

bool RetouchEditor::changed() const
{
    return IsChanged;
}

int RetouchEditor::imageWidth() const
{
    return CurrentImage.width();
}

int RetouchEditor::imageHeight() const
{
    return CurrentImage.height();
}

void RetouchEditor::openImage(const QString &image_file)
{
    int         img_orientation = 0;
    ExifLoader *exif_loader     = exif_loader_new();

    if (exif_loader != NULL) {
        ExifData *exif_data;

        exif_loader_write_file(exif_loader, image_file.toUtf8().data());

        exif_data = exif_loader_get_data(exif_loader);

        if (exif_data != NULL) {
            ExifByteOrder exif_bo    = exif_data_get_byte_order(exif_data);
            ExifEntry    *exif_entry = exif_content_get_entry(exif_data->ifd[EXIF_IFD_0], EXIF_TAG_ORIENTATION);

            if (exif_entry != NULL) {
                img_orientation = exif_get_short(exif_entry->data, exif_bo);

                exif_entry_unref(exif_entry);
            }

            exif_data_unref(exif_data);
        }

        exif_loader_unref(exif_loader);
    }

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
            if (img_orientation == 3) {
                QTransform transform;

                transform.rotate(180);

                LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.width(), LoadedImage.height());
            } else if (img_orientation == 6) {
                QTransform transform;

                transform.rotate(90);

                LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
            } else if (img_orientation == 8) {
                QTransform transform;

                transform.rotate(270);

                LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
            }

            LoadedImage = LoadedImage.convertToFormat(QImage::Format_RGB16);

            if (!LoadedImage.isNull()) {
                CurrentImage = LoadedImage;

                LoadedImage = QImage();

                UndoStack.clear();

                IsChanged = false;

                RepaintImage(true);

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
}

void RetouchEditor::saveImage(const QString &image_file)
{
    QString file_name = image_file;

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
}

void RetouchEditor::changeImageAt(bool save_undo, int center_x, int center_y, qreal zoom_level)
{
    if (CurrentMode == ModeClone || CurrentMode == ModeBlur) {
        if (save_undo) {
            SaveUndoImage();
        }

        int radius = BrushSize / zoom_level;

        if (CurrentMode == ModeClone) {
            for (int from_x = SamplingPoint.x() - radius, to_x = center_x - radius; from_x <= SamplingPoint.x() + radius && to_x <= center_x + radius; from_x++, to_x++) {
                for (int from_y = SamplingPoint.y() - radius, to_y = center_y - radius; from_y <= SamplingPoint.y() + radius && to_y <= center_y + radius; from_y++, to_y++) {
                    if (from_x >= 0 && from_x < CurrentImage.width() && from_y >= 0 && from_y < CurrentImage.height() && qSqrt(qPow(from_x - SamplingPoint.x(), 2) + qPow(from_y - SamplingPoint.y(), 2)) <= radius &&
                        to_x   >= 0 && to_x   < CurrentImage.width() && to_y   >= 0 && to_y   < CurrentImage.height() && qSqrt(qPow(to_x   - center_x,          2) + qPow(to_y   - center_y,          2)) <= radius) {
                        CurrentImage.setPixel(to_x, to_y, CurrentImage.pixel(from_x, from_y));
                    }
                }
            }
        } else if (CurrentMode == ModeBlur) {
            QRect  last_blur_rect(LastBlurPoint.x() - radius, LastBlurPoint.y() - radius, radius * 2, radius * 2);
            QImage last_blur_image;

            if (IsLastBlurPointValid) {
                if (last_blur_rect.x() >= CurrentImage.width()) {
                    last_blur_rect.setX(CurrentImage.width() - 1);
                }
                if (last_blur_rect.y() >= CurrentImage.height()) {
                    last_blur_rect.setY(CurrentImage.height() - 1);
                }
                if (last_blur_rect.x() < 0) {
                    last_blur_rect.setX(0);
                }
                if (last_blur_rect.y() < 0) {
                    last_blur_rect.setY(0);
                }
                if (last_blur_rect.x() + last_blur_rect.width() > CurrentImage.width()) {
                    last_blur_rect.setWidth(CurrentImage.width() - last_blur_rect.x());
                }
                if (last_blur_rect.y() + last_blur_rect.height() > CurrentImage.height()) {
                    last_blur_rect.setHeight(CurrentImage.height() - last_blur_rect.y());
                }

                last_blur_image = CurrentImage.copy(last_blur_rect);
            }

            QRect blur_rect(center_x - radius, center_y - radius, radius * 2, radius * 2);

            if (blur_rect.x() >= CurrentImage.width()) {
                blur_rect.setX(CurrentImage.width() - 1);
            }
            if (blur_rect.y() >= CurrentImage.height()) {
                blur_rect.setY(CurrentImage.height() - 1);
            }
            if (blur_rect.x() < 0) {
                blur_rect.setX(0);
            }
            if (blur_rect.y() < 0) {
                blur_rect.setY(0);
            }
            if (blur_rect.x() + blur_rect.width() > CurrentImage.width()) {
                blur_rect.setWidth(CurrentImage.width() - blur_rect.x());
            }
            if (blur_rect.y() + blur_rect.height() > CurrentImage.height()) {
                blur_rect.setHeight(CurrentImage.height() - blur_rect.y());
            }

            QImage blur_image = CurrentImage.copy(blur_rect).convertToFormat(QImage::Format_ARGB32_Premultiplied);

            int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
            int alpha = (GAUSSIAN_RADIUS < 1) ? 16 : (GAUSSIAN_RADIUS > 17) ? 1 : tab[GAUSSIAN_RADIUS - 1];

            int r1 = blur_image.rect().top();
            int r2 = blur_image.rect().bottom();
            int c1 = blur_image.rect().left();
            int c2 = blur_image.rect().right();

            int bpl = blur_image.bytesPerLine();

            int           rgba[4];
            unsigned char *p;

            for (int col = c1; col <= c2; col++) {
                p = blur_image.scanLine(r1) + col * 4;

                for (int i = 0; i < 4; i++) {
                    rgba[i] = p[i] << 4;
                }

                p += bpl;

                for (int j = r1; j < r2; j++, p += bpl) {
                    for (int i = 0; i < 4; i++) {
                        p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
                    }
                }
            }

            for (int row = r1; row <= r2; row++) {
                p = blur_image.scanLine(row) + c1 * 4;

                for (int i = 0; i < 4; i++) {
                    rgba[i] = p[i] << 4;
                }

                p += 4;

                for (int j = c1; j < c2; j++, p += 4) {
                    for (int i = 0; i < 4; i++) {
                        p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
                    }
                }
            }

            for (int col = c1; col <= c2; col++) {
                p = blur_image.scanLine(r2) + col * 4;

                for (int i = 0; i < 4; i++) {
                    rgba[i] = p[i] << 4;
                }

                p -= bpl;

                for (int j = r1; j < r2; j++, p -= bpl) {
                    for (int i = 0; i < 4; i++) {
                        p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
                    }
                }
            }

            for (int row = r1; row <= r2; row++) {
                p = blur_image.scanLine(row) + c2 * 4;

                for (int i = 0; i < 4; i++) {
                    rgba[i] = p[i] << 4;
                }

                p -= 4;

                for (int j = c1; j < c2; j++, p -= 4) {
                    for (int i = 0; i < 4; i++) {
                        p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
                    }
                }
            }

            QPainter painter(&CurrentImage);

            painter.setClipRegion(QRegion(blur_rect, QRegion::Ellipse));

            painter.drawImage(blur_rect, blur_image);

            if (IsLastBlurPointValid) {
                painter.setClipRegion(QRegion(last_blur_rect, QRegion::Ellipse));

                painter.drawImage(last_blur_rect, last_blur_image);
            }
        }

        IsChanged = true;

        RepaintImage(false, QRect(center_x - radius, center_y - radius, radius * 2, radius * 2));
        RepaintHelper(center_x, center_y, zoom_level);
    }
}

void RetouchEditor::updateHelperAt(int center_x, int center_y, qreal zoom_level)
{
    if (CurrentMode != ModeScroll) {
        RepaintHelper(center_x, center_y, zoom_level);
    }
}

void RetouchEditor::undo()
{
    if (UndoStack.size() > 0) {
        CurrentImage = UndoStack.pop();

        if (UndoStack.size() == 0) {
            emit undoAvailabilityChanged(false);
        }

        IsChanged = true;

        RepaintImage(true);
    }
}

void RetouchEditor::SaveUndoImage()
{
    UndoStack.push(CurrentImage);

    if (UndoStack.size() > UNDO_DEPTH) {
        for (int i = 0; i < UndoStack.size() - UNDO_DEPTH; i++) {
            UndoStack.remove(0);
        }
    }

    emit undoAvailabilityChanged(true);
}

void RetouchEditor::RepaintImage(bool full, QRect rect)
{
    if (CurrentImage.isNull()) {
        CurrentImageData = bb::ImageData();

        emit needImageRepaint(bb::cascades::Image());
    } else if (full) {
        CurrentImageData = bb::ImageData(bb::PixelFormat::RGBA_Premultiplied, CurrentImage.width(), CurrentImage.height());

        unsigned char *dst_line = CurrentImageData.pixels();

        for (int y = 0; y < CurrentImageData.height(); y++) {
            unsigned char *dst = dst_line;

            for (int x = 0; x < CurrentImageData.width(); x++) {
                QRgb pixel = CurrentImage.pixel(x, y);

                *dst++ = qRed(pixel);
                *dst++ = qGreen(pixel);
                *dst++ = qBlue(pixel);
                *dst++ = qAlpha(pixel);
            }

            dst_line += CurrentImageData.bytesPerLine();
        }

        emit needImageRepaint(bb::cascades::Image(CurrentImageData));
    } else {
        unsigned char *dst_line = CurrentImageData.pixels();

        if (rect.x() >= CurrentImageData.width()) {
            rect.setX(CurrentImageData.width() - 1);
        }
        if (rect.y() >= CurrentImageData.height()) {
            rect.setY(CurrentImageData.height() - 1);
        }
        if (rect.x() < 0) {
            rect.setX(0);
        }
        if (rect.y() < 0) {
            rect.setY(0);
        }
        if (rect.x() + rect.width() > CurrentImageData.width()) {
            rect.setWidth(CurrentImageData.width() - rect.x());
        }
        if (rect.y() + rect.height() > CurrentImageData.height()) {
            rect.setHeight(CurrentImageData.height() - rect.y());
        }

        dst_line += rect.y() * CurrentImageData.bytesPerLine();

        for (int y = rect.y(); y < rect.y() + rect.height(); y++) {
            unsigned char *dst = dst_line;

            dst += rect.x() * 4;

            for (int x = rect.x(); x < rect.x() + rect.width(); x++) {
                QRgb pixel = CurrentImage.pixel(x, y);

                *dst++ = qRed(pixel);
                *dst++ = qGreen(pixel);
                *dst++ = qBlue(pixel);
                *dst++ = qAlpha(pixel);
            }

            dst_line += CurrentImageData.bytesPerLine();
        }

        emit needImageRepaint(bb::cascades::Image(CurrentImageData));
    }
}

void RetouchEditor::RepaintHelper(int center_x, int center_y, qreal zoom_level)
{
    if (CurrentImage.isNull()) {
        emit needHelperRepaint(bb::cascades::Image());
    } else {
        QImage   helper_image = CurrentImage.copy(center_x - HelperSize / (zoom_level * 2),
                                                  center_y - HelperSize / (zoom_level * 2), HelperSize / zoom_level, HelperSize / zoom_level).scaledToWidth(HelperSize);
        QPainter painter(&helper_image);

        painter.setPen(QPen(Qt::white, 4, Qt::SolidLine));
        painter.drawPoint(helper_image.rect().center());

        bb::ImageData helper_image_data = bb::ImageData(bb::PixelFormat::RGBA_Premultiplied, helper_image.width(), helper_image.height());

        unsigned char *dst_line = helper_image_data.pixels();

        for (int y = 0; y < helper_image_data.height(); y++) {
            unsigned char *dst = dst_line;

            for (int x = 0; x < helper_image_data.width(); x++) {
                QRgb pixel = helper_image.pixel(x, y);

                *dst++ = qRed(pixel);
                *dst++ = qGreen(pixel);
                *dst++ = qBlue(pixel);
                *dst++ = qAlpha(pixel);
            }

            dst_line += helper_image_data.bytesPerLine();
        }

        emit needHelperRepaint(bb::cascades::Image(helper_image_data));
    }
}
