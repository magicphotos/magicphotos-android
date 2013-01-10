#include <QtCore/qmath.h>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QThread>
#include <QtGui/QImageReader>
#include <QtGui/QPainter>

#include "sketcheditor.h"

SketchEditor::SketchEditor() : bb::cascades::CustomControl()
{
    IsChanged      = false;
    CurrentMode    = ModeScroll;
    GaussianRadius = 0;
}

SketchEditor::~SketchEditor()
{
}

int SketchEditor::mode() const
{
    return CurrentMode;
}

void SketchEditor::setMode(const int &mode)
{
    CurrentMode = mode;
}

int SketchEditor::radius() const
{
    return GaussianRadius;
}

void SketchEditor::setRadius(const int &radius)
{
    GaussianRadius = radius;
}

bool SketchEditor::changed() const
{
    return IsChanged;
}

void SketchEditor::openImage(const QString &image_file)
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
                QThread              *thread    = new QThread();
                SketchImageGenerator *generator = new SketchImageGenerator();

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

void SketchEditor::saveImage(const QString &image_file)
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

void SketchEditor::changeImageAt(bool save_undo, int center_x, int center_y, double zoom_level)
{
    if (CurrentMode != ModeScroll) {
        if (save_undo) {
            SaveUndoImage();
        }

        int radius = BRUSH_SIZE / zoom_level;

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

void SketchEditor::undo()
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

void SketchEditor::effectedImageReady(const QImage &effected_image)
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

void SketchEditor::SaveUndoImage()
{
    UndoStack.push(CurrentImage);

    if (UndoStack.size() > UNDO_DEPTH) {
        for (int i = 0; i < UndoStack.size() - UNDO_DEPTH; i++) {
            UndoStack.remove(0);
        }
    }

    emit undoAvailabilityChanged(true);
}

void SketchEditor::RepaintImage(bool full, QRect rect)
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

void SketchEditor::RepaintHelper(int center_x, int center_y, double zoom_level)
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

SketchPreviewGenerator::SketchPreviewGenerator() : bb::cascades::CustomControl()
{
    GaussianRadius = 0;
}

SketchPreviewGenerator::~SketchPreviewGenerator()
{
}

int SketchPreviewGenerator::radius() const
{
    return GaussianRadius;
}

void SketchPreviewGenerator::setRadius(const int &radius)
{
    GaussianRadius = radius;

    if (!LoadedImage.isNull()) {
        QThread              *thread    = new QThread();
        SketchImageGenerator *generator = new SketchImageGenerator();

        generator->moveToThread(thread);

        QObject::connect(thread,    SIGNAL(started()),                  generator, SLOT(start()));
        QObject::connect(thread,    SIGNAL(finished()),                 thread,    SLOT(deleteLater()));
        QObject::connect(generator, SIGNAL(imageReady(const QImage &)), this,      SLOT(sketchImageReady(const QImage &)));
        QObject::connect(generator, SIGNAL(finished()),                 thread,    SLOT(quit()));
        QObject::connect(generator, SIGNAL(finished()),                 generator, SLOT(deleteLater()));

        generator->setGaussianRadius(GaussianRadius);
        generator->setInput(LoadedImage);

        thread->start();

        emit generationStarted();
    }
}

void SketchPreviewGenerator::openImage(const QString &image_file)
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
                QThread              *thread    = new QThread();
                SketchImageGenerator *generator = new SketchImageGenerator();

                generator->moveToThread(thread);

                QObject::connect(thread,    SIGNAL(started()),                  generator, SLOT(start()));
                QObject::connect(thread,    SIGNAL(finished()),                 thread,    SLOT(deleteLater()));
                QObject::connect(generator, SIGNAL(imageReady(const QImage &)), this,      SLOT(sketchImageReady(const QImage &)));
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

void SketchPreviewGenerator::sketchImageReady(const QImage &sketch_image)
{
    SketchImage = sketch_image;

    Repaint();

    emit generationFinished();
}

void SketchPreviewGenerator::Repaint()
{
    if (SketchImage.isNull()) {
        emit needRepaint(bb::cascades::Image());
    } else {
        bb::ImageData image_data = bb::ImageData(bb::PixelFormat::RGBA_Premultiplied, SketchImage.width(), SketchImage.height());

        unsigned char *dst_line = image_data.pixels();

        for (int y = 0; y < image_data.height(); y++) {
            unsigned char *dst = dst_line;

            for (int x = 0; x < image_data.width(); x++) {
                QRgb pixel = SketchImage.pixel(x, y);

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

SketchImageGenerator::SketchImageGenerator(QObject *parent) : QObject(parent)
{
    GaussianRadius = 0;
}

SketchImageGenerator::~SketchImageGenerator()
{
}

void SketchImageGenerator::setGaussianRadius(const int &radius)
{
    GaussianRadius = radius;
}

void SketchImageGenerator::setInput(const QImage &input_image)
{
    InputImage = input_image;
}

void SketchImageGenerator::start()
{
    QImage grayscale_image = InputImage;
    QImage sketch_image    = InputImage;

    // Make Gaussian blur of original image

    QImage::Format format = sketch_image.format();

    sketch_image = sketch_image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
    int alpha = (GaussianRadius < 1) ? 16 : (GaussianRadius > 17) ? 1 : tab[GaussianRadius - 1];

    int r1 = sketch_image.rect().top();
    int r2 = sketch_image.rect().bottom();
    int c1 = sketch_image.rect().left();
    int c2 = sketch_image.rect().right();

    int bpl = sketch_image.bytesPerLine();

    int           rgba[4];
    unsigned char *p;

    for (int col = c1; col <= c2; col++) {
        p = sketch_image.scanLine(r1) + col * 4;

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
        p = sketch_image.scanLine(row) + c1 * 4;

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
        p = sketch_image.scanLine(r2) + col * 4;

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
        p = sketch_image.scanLine(row) + c2 * 4;

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

    sketch_image = sketch_image.convertToFormat(format);

    // Make grayscale image from original image & inverted grayscale from blurred

    for (int x = 0; x < InputImage.width(); x++) {
        for (int y = 0; y < InputImage.height(); y++) {
            int gray  = qGray(InputImage.pixel(x, y));
            int alpha = qAlpha(InputImage.pixel(x, y));

            grayscale_image.setPixel(x, y, qRgba(gray, gray, gray, alpha));

            int blr_gray = qGray(sketch_image.pixel(x, y));
            int inv_gray = blr_gray >= 255 ? 0 : 255 - blr_gray;

            sketch_image.setPixel(x, y, qRgba(inv_gray, inv_gray, inv_gray, alpha));
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
    }

    emit imageReady(sketch_image);
    emit finished();
}
