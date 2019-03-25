#include <QtCore/QtMath>
#include <QtCore/QThread>
#include <QtGui/QColor>
#include <QtGui/QPainter>

#include "decolorizeeditor.h"

DecolorizeEditor::DecolorizeEditor(QQuickItem *parent) : EffectEditor(parent)
{
}

void DecolorizeEditor::processOpenedImage()
{
    auto thread    = new QThread();
    auto generator = new GrayscaleImageGenerator();

    generator->moveToThread(thread);

    QObject::connect(thread,    &QThread::started,                    generator, &GrayscaleImageGenerator::start);
    QObject::connect(thread,    &QThread::finished,                   thread,    &QThread::deleteLater);
    QObject::connect(generator, &GrayscaleImageGenerator::imageReady, this,      &DecolorizeEditor::effectedImageReady);
    QObject::connect(generator, &GrayscaleImageGenerator::finished,   thread,    &QThread::quit);
    QObject::connect(generator, &GrayscaleImageGenerator::finished,   generator, &GrayscaleImageGenerator::deleteLater);

    generator->setInput(LoadedImage);

    thread->start(QThread::LowPriority);
}

GrayscaleImageGenerator::GrayscaleImageGenerator(QObject *parent) : QObject(parent)
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
