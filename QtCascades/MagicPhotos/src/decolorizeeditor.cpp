#include <QtCore/qmath.h>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QThread>
#include <QtGui/QTransform>
#include <QtGui/QImageReader>
#include <QtGui/QPainter>

#include <libexif/exif-loader.h>
#include <libexif/exif-data.h>
#include <libexif/exif-entry.h>

#include "decolorizeeditor.h"

DecolorizeEditor::DecolorizeEditor() : bb::cascades::CustomControl()
{
    IsChanged   = false;
    CurrentMode = ModeScroll;
    BrushSize   = 0;
    HelperSize  = 0;
}

DecolorizeEditor::~DecolorizeEditor()
{
}

int DecolorizeEditor::mode() const
{
    return CurrentMode;
}

void DecolorizeEditor::setMode(const int &mode)
{
    CurrentMode = mode;
}

int DecolorizeEditor::brushSize() const
{
    return BrushSize;
}

void DecolorizeEditor::setBrushSize(const int &size)
{
    BrushSize = size;
}

int DecolorizeEditor::helperSize() const
{
    return HelperSize;
}

void DecolorizeEditor::setHelperSize(const int &size)
{
    HelperSize = size;
}

bool DecolorizeEditor::changed() const
{
    return IsChanged;
}

void DecolorizeEditor::openImage(const QString &image_file)
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
                QThread                 *thread    = new QThread();
                GrayscaleImageGenerator *generator = new GrayscaleImageGenerator();

                generator->moveToThread(thread);

                QObject::connect(thread,    SIGNAL(started()),                  generator, SLOT(start()));
                QObject::connect(thread,    SIGNAL(finished()),                 thread,    SLOT(deleteLater()));
                QObject::connect(generator, SIGNAL(imageReady(const QImage &)), this,      SLOT(effectedImageReady(const QImage &)));
                QObject::connect(generator, SIGNAL(finished()),                 thread,    SLOT(quit()));
                QObject::connect(generator, SIGNAL(finished()),                 generator, SLOT(deleteLater()));

                generator->setInput(LoadedImage);

                thread->start();
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

void DecolorizeEditor::saveImage(const QString &image_file)
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

void DecolorizeEditor::changeImageAt(bool save_undo, int center_x, int center_y, double zoom_level)
{
    if (CurrentMode != ModeScroll) {
        if (save_undo) {
            SaveUndoImage();
        }

        int radius = BrushSize / zoom_level;

        for (int x = center_x - radius; x <= center_x + radius; x++) {
            for (int y = center_y - radius; y <= center_y + radius; y++) {
                if (x >= 0 && x < CurrentImage.width() && y >= 0 && y < CurrentImage.height() && qSqrt(qPow(x - center_x, 2) + qPow(y - center_y, 2)) <= radius) {
                    if (CurrentMode == ModeOriginal) {
                        CurrentImage.setPixel(x, y, OriginalImage.pixel(x, y));
                    } else {
                        CurrentImage.setPixel(x, y, EffectedImage.pixel(x, y));
                    }
                }
            }
        }

        IsChanged = true;

        RepaintImage(false, QRect(center_x - radius, center_y - radius, radius * 2, radius * 2));
        RepaintHelper(center_x, center_y, zoom_level);
    }
}

void DecolorizeEditor::undo()
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

void DecolorizeEditor::effectedImageReady(const QImage &effected_image)
{
    OriginalImage = LoadedImage;
    EffectedImage = effected_image;
    CurrentImage  = EffectedImage;

    LoadedImage = QImage();

    UndoStack.clear();

    IsChanged = true;

    RepaintImage(true);

    emit undoAvailabilityChanged(false);
    emit imageOpened();
}

void DecolorizeEditor::SaveUndoImage()
{
    UndoStack.push(CurrentImage);

    if (UndoStack.size() > UNDO_DEPTH) {
        for (int i = 0; i < UndoStack.size() - UNDO_DEPTH; i++) {
            UndoStack.remove(0);
        }
    }

    emit undoAvailabilityChanged(true);
}

void DecolorizeEditor::RepaintImage(bool full, QRect rect)
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

void DecolorizeEditor::RepaintHelper(int center_x, int center_y, double zoom_level)
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

GrayscaleImageGenerator::GrayscaleImageGenerator(QObject *parent) : QObject(parent)
{
}

GrayscaleImageGenerator::~GrayscaleImageGenerator()
{
}

void GrayscaleImageGenerator::setInput(const QImage &input_image)
{
    InputImage = input_image;
}

void GrayscaleImageGenerator::start()
{
    QImage grayscale_image = InputImage;

    for (int x = 0; x < grayscale_image.width(); x++) {
        for (int y = 0; y < grayscale_image.height(); y++) {
            int gray  = qGray(grayscale_image.pixel(x, y));
            int alpha = qAlpha(grayscale_image.pixel(x, y));

            grayscale_image.setPixel(x, y, qRgba(gray, gray, gray, alpha));
        }
    }

    emit imageReady(grayscale_image);
    emit finished();
}
