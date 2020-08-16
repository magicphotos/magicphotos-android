#include <memory>

#include <QtCore/QtGlobal>
#include <QtCore/QThread>
#include <QtGui/QColor>
#include <QtGui/QPainter>

#include "sketcheditor.h"

SketchEditor::SketchEditor(QQuickItem *parent) :
    EffectEditor(parent),
    Radius      (0)
{
}

int SketchEditor::radius() const
{
    return Radius;
}

void SketchEditor::setRadius(int radius)
{
    Radius = radius;
}

void SketchEditor::ProcessOpenedImage()
{
    auto thread    = std::make_unique<QThread>();
    auto generator = std::make_unique<SketchImageGenerator>();

    generator->moveToThread(thread.get());

    connect(thread.get(),    &QThread::started,                 generator.get(), &SketchImageGenerator::start);
    connect(thread.get(),    &QThread::finished,                thread.get(),    &QThread::deleteLater);
    connect(generator.get(), &SketchImageGenerator::imageReady, this,            &SketchEditor::setEffectedImage);
    connect(generator.get(), &SketchImageGenerator::finished,   thread.get(),    &QThread::quit);
    connect(generator.get(), &SketchImageGenerator::finished,   generator.get(), &SketchImageGenerator::deleteLater);

    generator->SetRadius(Radius);
    generator->SetInput(LoadedImage);

    thread->start(QThread::LowPriority);

    static_cast<void>(thread.release());
    static_cast<void>(generator.release());
}

SketchPreviewGenerator::SketchPreviewGenerator(QQuickItem *parent) :
    PreviewGenerator(parent),
    Radius          (0)
{
}

int SketchPreviewGenerator::radius() const
{
    return Radius;
}

void SketchPreviewGenerator::setRadius(int radius)
{
    Radius = radius;

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
    auto thread    = std::make_unique<QThread>();
    auto generator = std::make_unique<SketchImageGenerator>();

    generator->moveToThread(thread.get());

    connect(thread.get(),    &QThread::started,                 generator.get(), &SketchImageGenerator::start);
    connect(thread.get(),    &QThread::finished,                thread.get(),    &QThread::deleteLater);
    connect(generator.get(), &SketchImageGenerator::imageReady, this,            &SketchPreviewGenerator::setEffectedImage);
    connect(generator.get(), &SketchImageGenerator::finished,   thread.get(),    &QThread::quit);
    connect(generator.get(), &SketchImageGenerator::finished,   generator.get(), &SketchImageGenerator::deleteLater);

    generator->SetRadius(Radius);
    generator->SetInput(LoadedImage);

    thread->start(QThread::LowPriority);

    static_cast<void>(thread.release());
    static_cast<void>(generator.release());

    ImageGeneratorRunning = true;

    emit generationStarted();
}

SketchImageGenerator::SketchImageGenerator(QObject *parent) :
    QObject(parent),
    Radius (0)
{
}

void SketchImageGenerator::SetRadius(int radius)
{
    Radius = radius;
}

void SketchImageGenerator::SetInput(const QImage &input_image)
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

    constexpr int tab[] = {14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2};

    int alpha = Radius < 1 ? 16 : (Radius > 17 ? 1 : tab[Radius - 1]);

    int r1 = sketch_image.rect().top();
    int r2 = sketch_image.rect().bottom();
    int c1 = sketch_image.rect().left();
    int c2 = sketch_image.rect().right();

    int bpl = sketch_image.bytesPerLine();

    int            rgba[4];
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
            int gray = qGray(InputImage.pixel(x, y));

            grayscale_image.setPixel(x, y, qRgba(gray, gray, gray, qAlpha(InputImage.pixel(x, y))));

            int blr_gray = qGray(sketch_image.pixel(x, y));
            int inv_gray = blr_gray >= 255 ? 0 : 255 - blr_gray;

            sketch_image.setPixel(x, y, qRgba(inv_gray, inv_gray, inv_gray, qAlpha(sketch_image.pixel(x, y))));
        }
    }

    // Apply Color Dodge mixing to grayscale & inverted blurred grayscale images for sketch

    for (int y = 0; y < sketch_image.height(); y++) {
        for (int x = 0; x < sketch_image.width(); x++) {
            int top_gray = qGray(sketch_image.pixel(x, y));
            int btm_gray = qGray(grayscale_image.pixel(x, y));
            int res_gray = top_gray >= 255 ? 255 : qMin(btm_gray * 255 / (255 - top_gray), 255);

            sketch_image.setPixel(x, y, qRgba(res_gray, res_gray, res_gray, qAlpha(sketch_image.pixel(x, y))));
        }
    }

    emit imageReady(sketch_image);
    emit finished();
}
