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

#include "blureditor.h"

BlurEditor::BlurEditor() : bb::cascades::CustomControl()
{
    IsChanged      = false;
    CurrentMode    = ModeScroll;
    BrushSize      = 0;
    HelperSize     = 0;
    GaussianRadius = 0;
}

BlurEditor::~BlurEditor()
{
}

int BlurEditor::mode() const
{
    return CurrentMode;
}

void BlurEditor::setMode(const int &mode)
{
    CurrentMode = mode;
}

int BlurEditor::brushSize() const
{
    return BrushSize;
}

void BlurEditor::setBrushSize(const int &size)
{
    BrushSize = size;
}

int BlurEditor::helperSize() const
{
    return HelperSize;
}

void BlurEditor::setHelperSize(const int &size)
{
    HelperSize = size;
}

int BlurEditor::radius() const
{
    return GaussianRadius;
}

void BlurEditor::setRadius(const int &radius)
{
    GaussianRadius = radius;
}

bool BlurEditor::changed() const
{
    return IsChanged;
}

void BlurEditor::openImage(const QString &image_file)
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
                QThread            *thread    = new QThread();
                BlurImageGenerator *generator = new BlurImageGenerator();

                generator->moveToThread(thread);

                QObject::connect(thread,    SIGNAL(started()),                  generator, SLOT(start()));
                QObject::connect(thread,    SIGNAL(finished()),                 thread,    SLOT(deleteLater()));
                QObject::connect(generator, SIGNAL(imageReady(const QImage &)), this,      SLOT(effectedImageReady(const QImage &)));
                QObject::connect(generator, SIGNAL(finished()),                 thread,    SLOT(quit()));
                QObject::connect(generator, SIGNAL(finished()),                 generator, SLOT(deleteLater()));

                generator->setGaussianRadius(GaussianRadius);
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

void BlurEditor::saveImage(const QString &image_file)
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

void BlurEditor::changeImageAt(bool save_undo, int center_x, int center_y, double zoom_level)
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

void BlurEditor::undo()
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

void BlurEditor::effectedImageReady(const QImage &effected_image)
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

void BlurEditor::SaveUndoImage()
{
    UndoStack.push(CurrentImage);

    if (UndoStack.size() > UNDO_DEPTH) {
        for (int i = 0; i < UndoStack.size() - UNDO_DEPTH; i++) {
            UndoStack.remove(0);
        }
    }

    emit undoAvailabilityChanged(true);
}

void BlurEditor::RepaintImage(bool full, QRect rect)
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

void BlurEditor::RepaintHelper(int center_x, int center_y, double zoom_level)
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

BlurPreviewGenerator::BlurPreviewGenerator() : bb::cascades::CustomControl()
{
    GaussianRadius = 0;
}

BlurPreviewGenerator::~BlurPreviewGenerator()
{
}

int BlurPreviewGenerator::radius() const
{
    return GaussianRadius;
}

void BlurPreviewGenerator::setRadius(const int &radius)
{
    GaussianRadius = radius;

    if (!LoadedImage.isNull()) {
        QThread            *thread    = new QThread();
        BlurImageGenerator *generator = new BlurImageGenerator();

        generator->moveToThread(thread);

        QObject::connect(thread,    SIGNAL(started()),                  generator, SLOT(start()));
        QObject::connect(thread,    SIGNAL(finished()),                 thread,    SLOT(deleteLater()));
        QObject::connect(generator, SIGNAL(imageReady(const QImage &)), this,      SLOT(blurImageReady(const QImage &)));
        QObject::connect(generator, SIGNAL(finished()),                 thread,    SLOT(quit()));
        QObject::connect(generator, SIGNAL(finished()),                 generator, SLOT(deleteLater()));

        generator->setGaussianRadius(GaussianRadius);
        generator->setInput(LoadedImage);

        thread->start();

        emit generationStarted();
    }
}

void BlurPreviewGenerator::openImage(const QString &image_file)
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
                QThread            *thread    = new QThread();
                BlurImageGenerator *generator = new BlurImageGenerator();

                generator->moveToThread(thread);

                QObject::connect(thread,    SIGNAL(started()),                  generator, SLOT(start()));
                QObject::connect(thread,    SIGNAL(finished()),                 thread,    SLOT(deleteLater()));
                QObject::connect(generator, SIGNAL(imageReady(const QImage &)), this,      SLOT(blurImageReady(const QImage &)));
                QObject::connect(generator, SIGNAL(finished()),                 thread,    SLOT(quit()));
                QObject::connect(generator, SIGNAL(finished()),                 generator, SLOT(deleteLater()));

                generator->setGaussianRadius(GaussianRadius);
                generator->setInput(LoadedImage);

                thread->start();

                emit imageOpened();
                emit generationStarted();
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

void BlurPreviewGenerator::blurImageReady(const QImage &blur_image)
{
    BlurImage = blur_image;

    Repaint();

    emit generationFinished();
}

void BlurPreviewGenerator::Repaint()
{
    if (BlurImage.isNull()) {
        emit needRepaint(bb::cascades::Image());
    } else {
        bb::ImageData image_data = bb::ImageData(bb::PixelFormat::RGBA_Premultiplied, BlurImage.width(), BlurImage.height());

        unsigned char *dst_line = image_data.pixels();

        for (int y = 0; y < image_data.height(); y++) {
            unsigned char *dst = dst_line;

            for (int x = 0; x < image_data.width(); x++) {
                QRgb pixel = BlurImage.pixel(x, y);

                *dst++ = qRed(pixel);
                *dst++ = qGreen(pixel);
                *dst++ = qBlue(pixel);
                *dst++ = qAlpha(pixel);
            }

            dst_line += image_data.bytesPerLine();
        }

        emit needRepaint(bb::cascades::Image(image_data));
    }
}

BlurImageGenerator::BlurImageGenerator(QObject *parent) : QObject(parent)
{
    GaussianRadius = 0;
}

BlurImageGenerator::~BlurImageGenerator()
{
}

void BlurImageGenerator::setGaussianRadius(const int &radius)
{
    GaussianRadius = radius;
}

void BlurImageGenerator::setInput(const QImage &input_image)
{
    InputImage = input_image;
}

void BlurImageGenerator::start()
{
    QImage blur_image = InputImage;

    QImage::Format format = blur_image.format();

    blur_image = blur_image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
    int alpha = (GaussianRadius < 1) ? 16 : (GaussianRadius > 17) ? 1 : tab[GaussianRadius - 1];

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

    blur_image = blur_image.convertToFormat(format);

    emit imageReady(blur_image);
    emit finished();
}
