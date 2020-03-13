#include <memory>

#include <QtCore/QtMath>
#include <QtCore/QThread>
#include <QtGui/QColor>
#include <QtGui/QPainter>

#include "decolorizeeditor.h"

DecolorizeEditor::DecolorizeEditor(QQuickItem *parent) :
    EffectEditor(parent)
{
}

void DecolorizeEditor::processOpenedImage()
{
    auto thread    = std::make_unique<QThread>();
    auto generator = std::make_unique<GrayscaleImageGenerator>();

    generator->moveToThread(thread.get());

    connect(thread.get(),    &QThread::started,                    generator.get(), &GrayscaleImageGenerator::start);
    connect(thread.get(),    &QThread::finished,                   thread.get(),    &QThread::deleteLater);
    connect(generator.get(), &GrayscaleImageGenerator::imageReady, this,            &DecolorizeEditor::setEffectedImage);
    connect(generator.get(), &GrayscaleImageGenerator::finished,   thread.get(),    &QThread::quit);
    connect(generator.get(), &GrayscaleImageGenerator::finished,   generator.get(), &GrayscaleImageGenerator::deleteLater);

    generator->setInput(LoadedImage);

    thread->start(QThread::LowPriority);

    static_cast<void>(thread.release());
    static_cast<void>(generator.release());
}

GrayscaleImageGenerator::GrayscaleImageGenerator(QObject *parent) :
    QObject(parent)
{
}

void GrayscaleImageGenerator::setInput(const QImage &input_image)
{
    InputImage = input_image;
}

void GrayscaleImageGenerator::start()
{
    QImage grayscale_image = InputImage;

    for (int y = 0; y < grayscale_image.height(); y++) {
        for (int x = 0; x < grayscale_image.width(); x++) {
            int gray  = qGray(grayscale_image.pixel(x, y));
            int alpha = qAlpha(grayscale_image.pixel(x, y));

            grayscale_image.setPixel(x, y, qRgba(gray, gray, gray, alpha));
        }
    }

    emit imageReady(grayscale_image);
    emit finished();
}
