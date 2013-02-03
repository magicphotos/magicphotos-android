#include <QtCore/qmath.h>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtGui/QImageReader>
#include <QtGui/QPainter>

#include "retoucheditor.h"

RetouchEditor::RetouchEditor() : bb::cascades::CustomControl()
{
    IsChanged            = false;
    IsSamplingPointValid = false;
    CurrentMode          = ModeScroll;
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

bool RetouchEditor::samplingPointValid() const
{
    return IsSamplingPointValid;
}

void RetouchEditor::setSamplingPointValid(const bool &valid)
{
    IsSamplingPointValid = valid;
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
                CurrentImage = LoadedImage;

                LoadedImage = QImage();

                UndoStack.clear();

                IsChanged = true;

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

void RetouchEditor::changeImageAt(bool save_undo, int center_x, int center_y, double zoom_level)
{
    if (CurrentMode == ModeClone) {
        if (save_undo) {
            SaveUndoImage();
        }

        int radius = BRUSH_SIZE / zoom_level;

        for (int from_x = SamplingPoint.x() - radius, to_x = center_x - radius; from_x <= SamplingPoint.x() + radius && to_x <= center_x + radius; from_x++, to_x++) {
            for (int from_y = SamplingPoint.y() - radius, to_y = center_y - radius; from_y <= SamplingPoint.y() + radius && to_y <= center_y + radius; from_y++, to_y++) {
                if (from_x >= 0 && from_x < CurrentImage.width() && from_y >= 0 && from_y < CurrentImage.height() && qSqrt(qPow(from_x - SamplingPoint.x(), 2) + qPow(from_y - SamplingPoint.y(), 2)) <= radius &&
                    to_x   >= 0 && to_x   < CurrentImage.width() && to_y   >= 0 && to_y   < CurrentImage.height() && qSqrt(qPow(to_x   - center_x,          2) + qPow(to_y   - center_y,          2)) <= radius) {
                    CurrentImage.setPixel(to_x, to_y, CurrentImage.pixel(from_x, from_y));
                }
            }
        }

        IsChanged = true;

        RepaintImage(false, QRect(center_x - radius, center_y - radius, radius * 2, radius * 2));
        RepaintHelper(center_x, center_y, zoom_level);
    }
}

void RetouchEditor::updateHelperAt(int center_x, int center_y, double zoom_level)
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

void RetouchEditor::RepaintHelper(int center_x, int center_y, double zoom_level)
{
    if (CurrentImage.isNull()) {
        emit needHelperRepaint(bb::cascades::Image());
    } else {
        QImage   helper_image = CurrentImage.copy(center_x - HELPER_SIZE / (zoom_level * 2),
                                                  center_y - HELPER_SIZE / (zoom_level * 2), HELPER_SIZE / zoom_level, HELPER_SIZE / zoom_level).scaledToWidth(HELPER_SIZE);
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
