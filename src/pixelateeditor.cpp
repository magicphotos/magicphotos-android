#include <QtCore/QtMath>
#include <QtCore/QThread>
#include <QtGui/QPainter>

#include "pixelateeditor.h"

PixelateEditor::PixelateEditor(QQuickItem *parent) : Editor(parent)
{
    PixelDenom = 0;
}

int PixelateEditor::pixDenom() const
{
    return PixelDenom;
}

void PixelateEditor::setPixDenom(int pix_denom)
{
    PixelDenom = pix_denom;
}

void PixelateEditor::effectedImageReady(const QImage &effected_image)
{
    OriginalImage = LoadedImage;
    EffectedImage = effected_image;
    CurrentImage  = EffectedImage;

    LoadedImage = QImage();

    IsChanged = true;

    setImplicitWidth(CurrentImage.width());
    setImplicitHeight(CurrentImage.height());

    update();

    emit scaleChanged();
    emit imageOpened();
}

void PixelateEditor::mousePressEvent(QMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        ChangeImageAt(true, event->pos().x(), event->pos().y());

        emit mouseEvent(MousePressed, event->pos().x(), event->pos().y());
    }
}

void PixelateEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        ChangeImageAt(false, event->pos().x(), event->pos().y());

        emit mouseEvent(MouseMoved, event->pos().x(), event->pos().y());
    }
}

void PixelateEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        emit mouseEvent(MouseReleased, event->pos().x(), event->pos().y());
    }
}

void PixelateEditor::processOpenedImage()
{
    auto thread    = new QThread();
    auto generator = new PixelateImageGenerator();

    generator->moveToThread(thread);

    QObject::connect(thread,    &QThread::started,                   generator, &PixelateImageGenerator::start);
    QObject::connect(thread,    &QThread::finished,                  thread,    &QThread::deleteLater);
    QObject::connect(generator, &PixelateImageGenerator::imageReady, this,      &PixelateEditor::effectedImageReady);
    QObject::connect(generator, &PixelateImageGenerator::finished,   thread,    &QThread::quit);
    QObject::connect(generator, &PixelateImageGenerator::finished,   generator, &PixelateImageGenerator::deleteLater);

    generator->setPixelDenom(PixelDenom);
    generator->setInput(LoadedImage);

    thread->start(QThread::LowPriority);
}

void PixelateEditor::ChangeImageAt(bool save_undo, int center_x, int center_y)
{
    if (CurrentMode != ModeScroll) {
        if (save_undo) {
            SaveUndoImage();
        }

        int width  = qMin(BrushImage.width(),  CurrentImage.width());
        int height = qMin(BrushImage.height(), CurrentImage.height());

        int img_x = qMin(qMax(0, center_x - width  / 2), CurrentImage.width()  - width);
        int img_y = qMin(qMax(0, center_y - height / 2), CurrentImage.height() - height);

        QImage   brush_image(width, height, QImage::Format_ARGB32);
        QPainter brush_painter(&brush_image);

        brush_painter.setCompositionMode(QPainter::CompositionMode_Source);

        if (CurrentMode == ModeOriginal) {
            brush_painter.drawImage(QPoint(0, 0), OriginalImage, QRect(img_x, img_y, width, height));
        } else {
            brush_painter.drawImage(QPoint(0, 0), EffectedImage, QRect(img_x, img_y, width, height));
        }

        QPainter image_painter(&CurrentImage);

        brush_painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        brush_painter.drawImage(QPoint(0, 0), BrushImage);

        image_painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        image_painter.drawImage(QPoint(img_x, img_y), brush_image);

        IsChanged = true;

        update();

        if (qFloor(HelperSize / scale()) > 0) {
            QImage helper_image = CurrentImage.copy(center_x - qFloor((HelperSize / scale()) / 2),
                                                    center_y - qFloor((HelperSize / scale()) / 2),
                                                    qFloor(HelperSize / scale()),
                                                    qFloor(HelperSize / scale())).scaledToWidth(HelperSize);

            emit helperImageReady(helper_image);
        } else {
            emit helperImageReady(QImage());
        }
    }
}

PixelatePreviewGenerator::PixelatePreviewGenerator(QQuickItem *parent) : PreviewGenerator(parent)
{
    PixelDenom = 0;
}

int PixelatePreviewGenerator::pixDenom() const
{
    return PixelDenom;
}

void PixelatePreviewGenerator::setPixDenom(int pix_denom)
{
    PixelDenom = pix_denom;

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
    auto thread    = new QThread();
    auto generator = new PixelateImageGenerator();

    generator->moveToThread(thread);

    QObject::connect(thread,    &QThread::started,                   generator, &PixelateImageGenerator::start);
    QObject::connect(thread,    &QThread::finished,                  thread,    &QThread::deleteLater);
    QObject::connect(generator, &PixelateImageGenerator::imageReady, this,      &PixelatePreviewGenerator::effectedImageReady);
    QObject::connect(generator, &PixelateImageGenerator::finished,   thread,    &QThread::quit);
    QObject::connect(generator, &PixelateImageGenerator::finished,   generator, &PixelateImageGenerator::deleteLater);

    generator->setPixelDenom(PixelDenom);
    generator->setInput(LoadedImage);

    thread->start(QThread::LowPriority);

    ImageGeneratorRunning = true;

    emit generationStarted();
}

PixelateImageGenerator::PixelateImageGenerator(QObject *parent) : QObject(parent)
{
    PixelDenom = 0;
}

void PixelateImageGenerator::setPixelDenom(int pix_denom)
{
    PixelDenom = pix_denom;
}

void PixelateImageGenerator::setInput(const QImage &input_image)
{
    InputImage = input_image;
}

void PixelateImageGenerator::start()
{
    QImage pixelated_image = InputImage;

    int pix_size = pixelated_image.width() > pixelated_image.height() ? pixelated_image.width() / PixelDenom : pixelated_image.height() / PixelDenom;

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
