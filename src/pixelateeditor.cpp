#include <memory>

#include <QtCore/QtMath>
#include <QtCore/QThread>
#include <QtGui/QColor>
#include <QtGui/QPainter>

#include "pixelateeditor.h"

PixelateEditor::PixelateEditor(QQuickItem *parent) :
    EffectEditor(parent),
    PixDenom    (0)
{
}

int PixelateEditor::pixDenom() const
{
    return PixDenom;
}

void PixelateEditor::setPixDenom(int pix_denom)
{
    PixDenom = pix_denom;
}

void PixelateEditor::ProcessOpenedImage()
{
    auto thread    = std::make_unique<QThread>();
    auto generator = std::make_unique<PixelateImageGenerator>();

    generator->moveToThread(thread.get());

    connect(thread.get(),    &QThread::started,                   generator.get(), &PixelateImageGenerator::start);
    connect(thread.get(),    &QThread::finished,                  thread.get(),    &QThread::deleteLater);
    connect(generator.get(), &PixelateImageGenerator::imageReady, this,            &PixelateEditor::setEffectedImage);
    connect(generator.get(), &PixelateImageGenerator::finished,   thread.get(),    &QThread::quit);
    connect(generator.get(), &PixelateImageGenerator::finished,   generator.get(), &PixelateImageGenerator::deleteLater);

    generator->SetPixDenom(PixDenom);
    generator->SetInput(LoadedImage);

    thread->start(QThread::LowPriority);

    static_cast<void>(thread.release());
    static_cast<void>(generator.release());
}

PixelatePreviewGenerator::PixelatePreviewGenerator(QQuickItem *parent) :
    PreviewGenerator(parent),
    PixDenom        (0)
{
}

int PixelatePreviewGenerator::pixDenom() const
{
    return PixDenom;
}

void PixelatePreviewGenerator::setPixDenom(int pix_denom)
{
    PixDenom = pix_denom;

    if (!LoadedImage.isNull()) {
        if (ImageGeneratorRunning) {
            RestartImageGenerator = true;
        } else {
            StartImageGenerator();
        }
    }
}

void PixelatePreviewGenerator::StartImageGenerator()
{
    auto thread    = std::make_unique<QThread>();
    auto generator = std::make_unique<PixelateImageGenerator>();

    generator->moveToThread(thread.get());

    connect(thread.get(),    &QThread::started,                   generator.get(), &PixelateImageGenerator::start);
    connect(thread.get(),    &QThread::finished,                  thread.get(),    &QThread::deleteLater);
    connect(generator.get(), &PixelateImageGenerator::imageReady, this,            &PixelatePreviewGenerator::setEffectedImage);
    connect(generator.get(), &PixelateImageGenerator::finished,   thread.get(),    &QThread::quit);
    connect(generator.get(), &PixelateImageGenerator::finished,   generator.get(), &PixelateImageGenerator::deleteLater);

    generator->SetPixDenom(PixDenom);
    generator->SetInput(LoadedImage);

    thread->start(QThread::LowPriority);

    static_cast<void>(thread.release());
    static_cast<void>(generator.release());

    ImageGeneratorRunning = true;

    emit generationStarted();
}

PixelateImageGenerator::PixelateImageGenerator(QObject *parent) :
    QObject (parent),
    PixDenom(0)
{
}

void PixelateImageGenerator::SetPixDenom(int pix_denom)
{
    PixDenom = pix_denom;
}

void PixelateImageGenerator::SetInput(const QImage &input_image)
{
    InputImage = input_image;
}

void PixelateImageGenerator::start()
{
    QImage pixelated_image = InputImage;

    int pix_size = pixelated_image.width() > pixelated_image.height() ? pixelated_image.width() / PixDenom : pixelated_image.height() / PixDenom;

    if (pix_size != 0) {
        for (int pix_y = 0; pix_y < pixelated_image.height() / pix_size + 1; pix_y++) {
            for (int pix_x = 0; pix_x < pixelated_image.width() / pix_size + 1; pix_x++) {
                int avg_r  = 0;
                int avg_g  = 0;
                int avg_b  = 0;
                int pixels = 0;

                for (int y = pix_y * pix_size; y < (pix_y + 1) * pix_size && y < pixelated_image.height(); y++) {
                    for (int x = pix_x * pix_size; x < (pix_x + 1) * pix_size && x < pixelated_image.width(); x++) {
                        QRgb pixel = pixelated_image.pixel(x, y);

                        avg_r += qRed(pixel);
                        avg_g += qGreen(pixel);
                        avg_b += qBlue(pixel);

                        pixels++;
                    }
                }

                if (pixels != 0) {
                    avg_r = avg_r / pixels;
                    avg_g = avg_g / pixels;
                    avg_b = avg_b / pixels;

                    for (int y = pix_y * pix_size; y < (pix_y + 1) * pix_size && y < pixelated_image.height(); y++) {
                        for (int x = pix_x * pix_size; x < (pix_x + 1) * pix_size && x < pixelated_image.width(); x++) {
                            pixelated_image.setPixel(x, y, qRgba(avg_r, avg_g, avg_b, qAlpha(pixelated_image.pixel(x, y))));
                        }
                    }
                }
            }
        }
    }

    emit imageReady(pixelated_image);
    emit finished();
}
