#include <QtCore/QThread>
#include <QtGui/QColor>
#include <QtGui/QPainter>

#include "sketcheditor.h"

SketchEditor::SketchEditor(QQuickItem *parent) : EffectEditor(parent)
{
    GaussianRadius = 0;
}

int SketchEditor::radius() const
{
    return GaussianRadius;
}

void SketchEditor::setRadius(int radius)
{
    GaussianRadius = radius;
}

void SketchEditor::processOpenedImage()
{
    auto thread    = new QThread();
    auto generator = new SketchImageGenerator();

    generator->moveToThread(thread);

    QObject::connect(thread,    &QThread::started,                 generator, &SketchImageGenerator::start);
    QObject::connect(thread,    &QThread::finished,                thread,    &QThread::deleteLater);
    QObject::connect(generator, &SketchImageGenerator::imageReady, this,      &SketchEditor::effectedImageReady);
    QObject::connect(generator, &SketchImageGenerator::finished,   thread,    &QThread::quit);
    QObject::connect(generator, &SketchImageGenerator::finished,   generator, &SketchImageGenerator::deleteLater);

    generator->setGaussianRadius(GaussianRadius);
    generator->setInput(LoadedImage);

    thread->start(QThread::LowPriority);
}

SketchPreviewGenerator::SketchPreviewGenerator(QQuickItem *parent) : PreviewGenerator(parent)
{
    GaussianRadius = 0;
}

int SketchPreviewGenerator::radius() const
{
    return GaussianRadius;
}

void SketchPreviewGenerator::setRadius(int radius)
{
    GaussianRadius = radius;

    if (!LoadedImage.isNull()) {
        if (ImageGeneratorRunning) {
            RestartImageGenerator = true;
        } else {
            StartImageGenerator();
        }
    }
}

void SketchPreviewGenerator::StartImageGenerator()
{
    auto thread    = new QThread();
    auto generator = new SketchImageGenerator();

    generator->moveToThread(thread);

    QObject::connect(thread,    &QThread::started,                 generator, &SketchImageGenerator::start);
    QObject::connect(thread,    &QThread::finished,                thread,    &QThread::deleteLater);
    QObject::connect(generator, &SketchImageGenerator::imageReady, this,      &SketchPreviewGenerator::effectedImageReady);
    QObject::connect(generator, &SketchImageGenerator::finished,   thread,    &QThread::quit);
    QObject::connect(generator, &SketchImageGenerator::finished,   generator, &SketchImageGenerator::deleteLater);

    generator->setGaussianRadius(GaussianRadius);
    generator->setInput(LoadedImage);

    thread->start(QThread::LowPriority);

    ImageGeneratorRunning = true;

    emit generationStarted();
}

SketchImageGenerator::SketchImageGenerator(QObject *parent) : QObject(parent)
{
    GaussianRadius = 0;
}

void SketchImageGenerator::setGaussianRadius(int radius)
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
                p[i] = static_cast<unsigned char>((rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4);
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
                p[i] = static_cast<unsigned char>((rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4);
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
                p[i] = static_cast<unsigned char>((rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4);
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
                p[i] = static_cast<unsigned char>((rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4);
            }
        }
    }

    sketch_image = sketch_image.convertToFormat(format);

    // Make grayscale image from original image & inverted grayscale from blurred

    for (int y = 0; y < InputImage.height(); y++) {
        for (int x = 0; x < InputImage.width(); x++) {
            int gray  = qGray(InputImage.pixel(x, y));
            int alpha = qAlpha(InputImage.pixel(x, y));

            grayscale_image.setPixel(x, y, qRgba(gray, gray, gray, alpha));

            int blr_gray = qGray(sketch_image.pixel(x, y));
            int inv_gray = blr_gray >= 255 ? 0 : 255 - blr_gray;

            sketch_image.setPixel(x, y, qRgba(inv_gray, inv_gray, inv_gray, alpha));
        }
    }

    // Apply Color Dodge mixing to grayscale & inverted blurred grayscale images for sketch

    for (int y = 0; y < sketch_image.height(); y++) {
        for (int x = 0; x < sketch_image.width(); x++) {
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
