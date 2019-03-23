#include <QtCore/QtMath>
#include <QtCore/QThread>
#include <QtGui/QPainter>

#include "blureditor.h"

BlurEditor::BlurEditor(QQuickItem *parent) : Editor(parent)
{
    GaussianRadius = 0;
}

int BlurEditor::radius() const
{
    return GaussianRadius;
}

void BlurEditor::setRadius(int radius)
{
    GaussianRadius = radius;
}

void BlurEditor::effectedImageReady(const QImage &effected_image)
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

void BlurEditor::mousePressEvent(QMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        ChangeImageAt(true, event->pos().x(), event->pos().y());

        emit mouseEvent(MousePressed, event->pos().x(), event->pos().y());
    }
}

void BlurEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        ChangeImageAt(false, event->pos().x(), event->pos().y());

        emit mouseEvent(MouseMoved, event->pos().x(), event->pos().y());
    }
}

void BlurEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        emit mouseEvent(MouseReleased, event->pos().x(), event->pos().y());
    }
}

void BlurEditor::processOpenedImage()
{
    auto thread    = new QThread();
    auto generator = new BlurImageGenerator();

    generator->moveToThread(thread);

    QObject::connect(thread,    &QThread::started,               generator, &BlurImageGenerator::start);
    QObject::connect(thread,    &QThread::finished,              thread,    &QThread::deleteLater);
    QObject::connect(generator, &BlurImageGenerator::imageReady, this,      &BlurEditor::effectedImageReady);
    QObject::connect(generator, &BlurImageGenerator::finished,   thread,    &QThread::quit);
    QObject::connect(generator, &BlurImageGenerator::finished,   generator, &BlurImageGenerator::deleteLater);

    generator->setGaussianRadius(GaussianRadius);
    generator->setInput(LoadedImage);

    thread->start(QThread::LowPriority);
}

void BlurEditor::ChangeImageAt(bool save_undo, int center_x, int center_y)
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

BlurPreviewGenerator::BlurPreviewGenerator(QQuickItem *parent) : PreviewGenerator(parent)
{
    GaussianRadius = 0;
}

int BlurPreviewGenerator::radius() const
{
    return GaussianRadius;
}

void BlurPreviewGenerator::setRadius(int radius)
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

void BlurPreviewGenerator::StartImageGenerator()
{
    auto thread    = new QThread();
    auto generator = new BlurImageGenerator();

    generator->moveToThread(thread);

    QObject::connect(thread,    &QThread::started,               generator, &BlurImageGenerator::start);
    QObject::connect(thread,    &QThread::finished,              thread,    &QThread::deleteLater);
    QObject::connect(generator, &BlurImageGenerator::imageReady, this,      &BlurPreviewGenerator::effectedImageReady);
    QObject::connect(generator, &BlurImageGenerator::finished,   thread,    &QThread::quit);
    QObject::connect(generator, &BlurImageGenerator::finished,   generator, &BlurImageGenerator::deleteLater);

    generator->setGaussianRadius(GaussianRadius);
    generator->setInput(LoadedImage);

    thread->start(QThread::LowPriority);

    ImageGeneratorRunning = true;

    emit generationStarted();
}

BlurImageGenerator::BlurImageGenerator(QObject *parent) : QObject(parent)
{
    GaussianRadius = 0;
}

void BlurImageGenerator::setGaussianRadius(int radius)
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
                p[i] = static_cast<unsigned char>((rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4);
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
                p[i] = static_cast<unsigned char>((rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4);
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
                p[i] = static_cast<unsigned char>((rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4);
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
                p[i] = static_cast<unsigned char>((rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4);
            }
        }
    }

    blur_image = blur_image.convertToFormat(format);

    emit imageReady(blur_image);
    emit finished();
}
