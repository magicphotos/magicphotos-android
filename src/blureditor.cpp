#include <QtCore/QtMath>
#include <QtCore/QFileInfo>
#include <QtCore/QThread>
#include <QtGui/QTransform>
#include <QtGui/QImageReader>
#include <QtGui/QPainter>

#include "blureditor.h"

BlurEditor::BlurEditor(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    IsChanged      = false;
    CurrentMode    = ModeScroll;
    BrushSize      = 0;
    HelperSize     = 0;
    GaussianRadius = 0;
    BrushOpacity   = 0.0;

    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);

    setFlag(QQuickItem::ItemHasContents, true);

    QObject::connect(this, &BlurEditor::scaleChanged, this, &BlurEditor::scaleWasChanged);
}

bool BlurEditor::changed() const
{
    return IsChanged;
}

int BlurEditor::mode() const
{
    return CurrentMode;
}

void BlurEditor::setMode(int mode)
{
    CurrentMode = mode;
}

int BlurEditor::brushSize() const
{
    return BrushSize;
}

void BlurEditor::setBrushSize(int size)
{
    BrushSize = size;

    BrushTemplateImage = QImage(BrushSize * 2, BrushSize * 2, QImage::Format_ARGB32);

    for (int x = 0; x < BrushTemplateImage.width(); x++) {
        for (int y = 0; y < BrushTemplateImage.height(); y++) {
            qreal r = qSqrt(qPow(x - BrushSize, 2) + qPow(y - BrushSize, 2));

            if (r <= BrushSize) {
                if (r <= BrushSize * BrushOpacity) {
                    BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, 0xFF));
                } else {
                    BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, qFloor(0xFF * (BrushSize - r) / (BrushSize * (1.0 - BrushOpacity)))));
                }
            } else {
                BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, 0x00));
            }
        }
    }

    int brush_width = qMax(1, qMin(qMin(qFloor(BrushSize / scale()) * 2, CurrentImage.width()), CurrentImage.height()));

    BrushImage = BrushTemplateImage.scaledToWidth(brush_width);
}

int BlurEditor::helperSize() const
{
    return HelperSize;
}

void BlurEditor::setHelperSize(int size)
{
    HelperSize = size;
}

int BlurEditor::radius() const
{
    return GaussianRadius;
}

void BlurEditor::setRadius(int radius)
{
    GaussianRadius = radius;
}

qreal BlurEditor::brushOpacity() const
{
    return BrushOpacity;
}

void BlurEditor::setBrushOpacity(qreal opacity)
{
    BrushOpacity = opacity;

    BrushTemplateImage = QImage(BrushSize * 2, BrushSize * 2, QImage::Format_ARGB32);

    for (int x = 0; x < BrushTemplateImage.width(); x++) {
        for (int y = 0; y < BrushTemplateImage.height(); y++) {
            qreal r = qSqrt(qPow(x - BrushSize, 2) + qPow(y - BrushSize, 2));

            if (r <= BrushSize) {
                if (r <= BrushSize * BrushOpacity) {
                    BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, 0xFF));
                } else {
                    BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, qFloor(0xFF * (BrushSize - r) / (BrushSize * (1.0 - BrushOpacity)))));
                }
            } else {
                BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, 0x00));
            }
        }
    }

    int brush_width = qMax(1, qMin(qMin(qFloor(BrushSize / scale()) * 2, CurrentImage.width()), CurrentImage.height()));

    BrushImage = BrushTemplateImage.scaledToWidth(brush_width);
}

void BlurEditor::openImage(const QString &image_file, int image_orientation)
{
    if (!image_file.isNull()) {
        QImageReader reader(image_file);

        if (reader.canRead()) {
            QSize size = reader.size();

            if (size.width() * size.height() > IMAGE_MPIX_LIMIT * 1000000.0) {
                qreal factor = qSqrt((size.width() * size.height()) / (IMAGE_MPIX_LIMIT * 1000000.0));

                size.setWidth(qFloor(size.width()   / factor));
                size.setHeight(qFloor(size.height() / factor));

                reader.setScaledSize(size);
            }

            LoadedImage = reader.read();

            if (!LoadedImage.isNull()) {
                if (image_orientation == 90) {
                    QTransform transform;

                    transform.rotate(90);

                    LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
                } else if (image_orientation == 180) {
                    QTransform transform;

                    transform.rotate(180);

                    LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.width(), LoadedImage.height());
                } else if (image_orientation == 270) {
                    QTransform transform;

                    transform.rotate(270);

                    LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
                }

                LoadedImage = LoadedImage.convertToFormat(QImage::Format_RGB32);

                if (!LoadedImage.isNull()) {
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
                } else {
                    emit imageOpenFailed();
                }
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

    if (!file_name.isNull()) {
        if (!CurrentImage.isNull()) {
            if (QFileInfo(file_name).suffix().compare("png", Qt::CaseInsensitive) != 0 &&
                QFileInfo(file_name).suffix().compare("jpg", Qt::CaseInsensitive) != 0 &&
                QFileInfo(file_name).suffix().compare("bmp", Qt::CaseInsensitive) != 0) {
                file_name = file_name + ".jpg";
            }

            if (CurrentImage.convertToFormat(QImage::Format_ARGB32).save(file_name)) {
                IsChanged = false;

                emit imageSaved(file_name);
            } else {
                emit imageSaveFailed();
            }
        } else {
            emit imageSaveFailed();
        }
    } else {
        emit imageSaveFailed();
    }
}

void BlurEditor::undo()
{
    if (UndoStack.count() > 0) {
        CurrentImage = UndoStack.pop();

        if (UndoStack.count() == 0) {
            emit undoAvailabilityChanged(false);
        }

        IsChanged = true;

        update();
    }
}

void BlurEditor::paint(QPainter *painter)
{
    painter->drawImage(QRectF(0, 0, width(), height()), CurrentImage, QRectF(0, 0, CurrentImage.width(), CurrentImage.height()));
}

void BlurEditor::effectedImageReady(const QImage &effected_image)
{
    OriginalImage = LoadedImage;
    EffectedImage = effected_image;
    CurrentImage  = EffectedImage;

    LoadedImage = QImage();

    UndoStack.clear();

    IsChanged = true;

    setImplicitWidth(CurrentImage.width());
    setImplicitHeight(CurrentImage.height());

    update();

    int brush_width = qMax(1, qMin(qMin(qFloor(BrushSize / scale()) * 2, CurrentImage.width()), CurrentImage.height()));

    BrushImage = BrushTemplateImage.scaledToWidth(brush_width);

    emit undoAvailabilityChanged(false);
    emit imageOpened();
}

void BlurEditor::scaleWasChanged()
{
    int brush_width = qMax(1, qMin(qMin(qFloor(BrushSize / scale()) * 2, CurrentImage.width()), CurrentImage.height()));

    BrushImage = BrushTemplateImage.scaledToWidth(brush_width);
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

void BlurEditor::SaveUndoImage()
{
    UndoStack.push(CurrentImage);

    if (UndoStack.count() > UNDO_DEPTH) {
        for (int i = 0; i < UndoStack.count() - UNDO_DEPTH; i++) {
            UndoStack.remove(0);
        }
    }

    emit undoAvailabilityChanged(true);
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

        QImage helper_image = CurrentImage.copy(center_x - qFloor((HelperSize / scale()) / 2),
                                                center_y - qFloor((HelperSize / scale()) / 2),
                                                qFloor(HelperSize / scale()),
                                                qFloor(HelperSize / scale())).scaledToWidth(HelperSize);

        emit helperImageReady(helper_image);
    }
}

BlurPreviewGenerator::BlurPreviewGenerator(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    BlurGeneratorRunning = false;
    RestartBlurGenerator = false;
    GaussianRadius       = 0;

    setFlag(QQuickItem::ItemHasContents, true);
}

int BlurPreviewGenerator::radius() const
{
    return GaussianRadius;
}

void BlurPreviewGenerator::setRadius(int radius)
{
    GaussianRadius = radius;

    if (!LoadedImage.isNull()) {
        if (BlurGeneratorRunning) {
            RestartBlurGenerator = true;
        } else {
            StartBlurGenerator();
        }
    }
}

void BlurPreviewGenerator::openImage(const QString &image_file, int image_orientation)
{
    if (!image_file.isNull()) {
        QImageReader reader(image_file);

        if (reader.canRead()) {
            QSize size = reader.size();

            if (size.width() * size.height() > IMAGE_MPIX_LIMIT * 1000000.0) {
                qreal factor = qSqrt((size.width() * size.height()) / (IMAGE_MPIX_LIMIT * 1000000.0));

                size.setWidth(qFloor(size.width()   / factor));
                size.setHeight(qFloor(size.height() / factor));

                reader.setScaledSize(size);
            }

            LoadedImage = reader.read();

            if (!LoadedImage.isNull()) {
                if (image_orientation == 90) {
                    QTransform transform;

                    transform.rotate(90);

                    LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
                } else if (image_orientation == 180) {
                    QTransform transform;

                    transform.rotate(180);

                    LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.width(), LoadedImage.height());
                } else if (image_orientation == 270) {
                    QTransform transform;

                    transform.rotate(270);

                    LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
                }

                LoadedImage = LoadedImage.convertToFormat(QImage::Format_RGB32);

                if (!LoadedImage.isNull()) {
                    emit imageOpened();

                    if (BlurGeneratorRunning) {
                        RestartBlurGenerator = true;
                    } else {
                        StartBlurGenerator();
                    }
                } else {
                    emit imageOpenFailed();
                }
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

void BlurPreviewGenerator::paint(QPainter *painter)
{
    if (!BlurImage.isNull()) {
        QImage image = BlurImage.scaled(QSize(qFloor(width()), qFloor(height())), Qt::KeepAspectRatio,
                                        smooth() ? Qt::SmoothTransformation : Qt::FastTransformation);

        painter->drawImage(QPointF((width()  - image.width())  / 2,
                                   (height() - image.height()) / 2), image);
    }
}

void BlurPreviewGenerator::blurImageReady(const QImage &blur_image)
{
    BlurGeneratorRunning = false;
    BlurImage            = blur_image;

    setImplicitWidth(BlurImage.width());
    setImplicitHeight(BlurImage.height());

    update();

    emit generationFinished();

    if (RestartBlurGenerator) {
        StartBlurGenerator();

        RestartBlurGenerator = false;
    }
}

void BlurPreviewGenerator::StartBlurGenerator()
{
    auto thread    = new QThread();
    auto generator = new BlurImageGenerator();

    generator->moveToThread(thread);

    QObject::connect(thread,    &QThread::started,               generator, &BlurImageGenerator::start);
    QObject::connect(thread,    &QThread::finished,              thread,    &QThread::deleteLater);
    QObject::connect(generator, &BlurImageGenerator::imageReady, this,      &BlurPreviewGenerator::blurImageReady);
    QObject::connect(generator, &BlurImageGenerator::finished,   thread,    &QThread::quit);
    QObject::connect(generator, &BlurImageGenerator::finished,   generator, &BlurImageGenerator::deleteLater);

    generator->setGaussianRadius(GaussianRadius);
    generator->setInput(LoadedImage);

    thread->start(QThread::LowPriority);

    BlurGeneratorRunning = true;

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
