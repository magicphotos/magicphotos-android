#include <QtCore/qmath.h>
#include <QtCore/QFileInfo>
#include <QtCore/QThread>
#include <QtGui/QTransform>
#include <QtGui/QImageReader>
#include <QtGui/QPainter>

#include "pixelateeditor.h"

PixelateEditor::PixelateEditor(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    IsChanged   = false;
    CurrentMode = ModeScroll;
    HelperSize  = 0;
    ScreenDPI   = 0;
    PixelDenom  = 0;

    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);

    setRenderTarget(QQuickPaintedItem::FramebufferObject);

    setFlag(QQuickItem::ItemHasContents, true);
}

PixelateEditor::~PixelateEditor()
{
}

int PixelateEditor::mode() const
{
    return CurrentMode;
}

void PixelateEditor::setMode(const int &mode)
{
    CurrentMode = mode;
}

int PixelateEditor::helperSize() const
{
    return HelperSize;
}

void PixelateEditor::setHelperSize(const int &size)
{
    HelperSize = size;
}

int PixelateEditor::screenDPI() const
{
    return ScreenDPI;
}

void PixelateEditor::setScreenDPI(const int &dpi)
{
    ScreenDPI = dpi;
}

int PixelateEditor::pixDenom() const
{
    return PixelDenom;
}

void PixelateEditor::setPixDenom(const int &pix_denom)
{
    PixelDenom = pix_denom;
}

bool PixelateEditor::changed() const
{
    return IsChanged;
}

void PixelateEditor::openImage(const QString &image_file, const int &image_orientation)
{
    if (!image_file.isNull()) {
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
                if (image_orientation == 3) {
                    QTransform transform;

                    transform.rotate(180);

                    LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.width(), LoadedImage.height());
                } else if (image_orientation == 6) {
                    QTransform transform;

                    transform.rotate(90);

                    LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
                } else if (image_orientation == 8) {
                    QTransform transform;

                    transform.rotate(270);

                    LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
                }

                LoadedImage = LoadedImage.convertToFormat(QImage::Format_RGB16);

                if (!LoadedImage.isNull()) {
                    QThread                *thread    = new QThread();
                    PixelateImageGenerator *generator = new PixelateImageGenerator();

                    generator->moveToThread(thread);

                    QObject::connect(thread,    SIGNAL(started()),                  generator, SLOT(start()));
                    QObject::connect(thread,    SIGNAL(finished()),                 thread,    SLOT(deleteLater()));
                    QObject::connect(generator, SIGNAL(imageReady(const QImage &)), this,      SLOT(effectedImageReady(const QImage &)));
                    QObject::connect(generator, SIGNAL(finished()),                 thread,    SLOT(quit()));
                    QObject::connect(generator, SIGNAL(finished()),                 generator, SLOT(deleteLater()));

                    generator->setPixelDenom(PixelDenom);
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

void PixelateEditor::saveImage(const QString &image_file)
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

void PixelateEditor::undo()
{
    if (UndoStack.size() > 0) {
        CurrentImage = UndoStack.pop();

        if (UndoStack.size() == 0) {
            emit undoAvailabilityChanged(false);
        }

        IsChanged = true;

        update();
    }
}

void PixelateEditor::paint(QPainter *painter)
{
    bool smooth_pixmap = painter->testRenderHint(QPainter::SmoothPixmapTransform);

    if (smooth()) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    } else {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
    }

    painter->drawPixmap(contentsBoundingRect(), QPixmap::fromImage(CurrentImage), QRectF(0, 0, CurrentImage.width(), CurrentImage.height()));

    painter->setRenderHint(QPainter::SmoothPixmapTransform, smooth_pixmap);
}

void PixelateEditor::effectedImageReady(const QImage &effected_image)
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

    emit undoAvailabilityChanged(false);
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

int PixelateEditor::MapSizeToDevice(int size)
{
    if (ScreenDPI >= 640) {
        return size * 4;
    } else if (ScreenDPI >= 480) {
        return size * 3;
    } else if (ScreenDPI >= 320) {
        return size * 2;
    } else if (ScreenDPI >= 240) {
        return size * 1.5;
    } else {
        return size;
    }
}

void PixelateEditor::SaveUndoImage()
{
    UndoStack.push(CurrentImage);

    if (UndoStack.size() > UNDO_DEPTH) {
        for (int i = 0; i < UndoStack.size() - UNDO_DEPTH; i++) {
            UndoStack.remove(0);
        }
    }

    emit undoAvailabilityChanged(true);
}

void PixelateEditor::ChangeImageAt(bool save_undo, int center_x, int center_y)
{
    if (CurrentMode != ModeScroll) {
        if (save_undo) {
            SaveUndoImage();
        }

        int radius = MapSizeToDevice(BRUSH_SIZE) / scale();

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

        update();

        QImage helper_image = CurrentImage.copy(center_x - (HelperSize / scale()) / 2,
                                                center_y - (HelperSize / scale()) / 2,
                                                HelperSize / scale(),
                                                HelperSize / scale()).scaledToWidth(HelperSize);

        emit helperImageReady(helper_image);
    }
}

PixelatePreviewGenerator::PixelatePreviewGenerator(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    PixelateGeneratorRunning = false;
    RestartPixelateGenerator = false;
    PixelDenom               = 0;

    setRenderTarget(QQuickPaintedItem::FramebufferObject);

    setFlag(QQuickItem::ItemHasContents, true);
}

PixelatePreviewGenerator::~PixelatePreviewGenerator()
{
}

int PixelatePreviewGenerator::pixDenom() const
{
    return PixelDenom;
}

void PixelatePreviewGenerator::setPixDenom(const int &pix_denom)
{
    PixelDenom = pix_denom;

    if (!LoadedImage.isNull()) {
        if (PixelateGeneratorRunning) {
            RestartPixelateGenerator = true;
        } else {
            StartPixelateGenerator();
        }
    }
}

void PixelatePreviewGenerator::openImage(const QString &image_file, const int &image_orientation)
{
    if (!image_file.isNull()) {
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
                if (image_orientation == 3) {
                    QTransform transform;

                    transform.rotate(180);

                    LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.width(), LoadedImage.height());
                } else if (image_orientation == 6) {
                    QTransform transform;

                    transform.rotate(90);

                    LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
                } else if (image_orientation == 8) {
                    QTransform transform;

                    transform.rotate(270);

                    LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
                }

                LoadedImage = LoadedImage.convertToFormat(QImage::Format_RGB16);

                if (!LoadedImage.isNull()) {
                    emit imageOpened();

                    if (PixelateGeneratorRunning) {
                        RestartPixelateGenerator = true;
                    } else {
                        StartPixelateGenerator();
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

void PixelatePreviewGenerator::paint(QPainter *painter)
{
    bool smooth_pixmap = painter->testRenderHint(QPainter::SmoothPixmapTransform);

    if (smooth()) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    } else {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
    }

    if (!PixelatedImage.isNull()) {
        QImage image = PixelatedImage.scaled(QSize(contentsBoundingRect().width(),
                                                   contentsBoundingRect().height()),
                                             Qt::KeepAspectRatio,
                                             smooth() ? Qt::SmoothTransformation : Qt::FastTransformation);

        painter->drawPixmap(QPoint((contentsBoundingRect().width()  - image.width())  / 2,
                                   (contentsBoundingRect().height() - image.height()) / 2), QPixmap::fromImage(image));
    }

    painter->setRenderHint(QPainter::SmoothPixmapTransform, smooth_pixmap);
}

void PixelatePreviewGenerator::pixelatedImageReady(const QImage &pixelated_image)
{
    PixelateGeneratorRunning = false;
    PixelatedImage           = pixelated_image;

    setImplicitWidth(PixelatedImage.width());
    setImplicitHeight(PixelatedImage.height());

    update();

    emit generationFinished();

    if (RestartPixelateGenerator) {
        StartPixelateGenerator();

        RestartPixelateGenerator = false;
    }
}

void PixelatePreviewGenerator::StartPixelateGenerator()
{
    QThread                *thread    = new QThread();
    PixelateImageGenerator *generator = new PixelateImageGenerator();

    generator->moveToThread(thread);

    QObject::connect(thread,    SIGNAL(started()),                  generator, SLOT(start()));
    QObject::connect(thread,    SIGNAL(finished()),                 thread,    SLOT(deleteLater()));
    QObject::connect(generator, SIGNAL(imageReady(const QImage &)), this,      SLOT(pixelatedImageReady(const QImage &)));
    QObject::connect(generator, SIGNAL(finished()),                 thread,    SLOT(quit()));
    QObject::connect(generator, SIGNAL(finished()),                 generator, SLOT(deleteLater()));

    generator->setPixelDenom(PixelDenom);
    generator->setInput(LoadedImage);

    thread->start(QThread::LowPriority);

    PixelateGeneratorRunning = true;

    emit generationStarted();
}

PixelateImageGenerator::PixelateImageGenerator(QObject *parent) : QObject(parent)
{
    PixelDenom = 0;
}

PixelateImageGenerator::~PixelateImageGenerator()
{
}

void PixelateImageGenerator::setPixelDenom(const int &pix_denom)
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
        for (int i = 0; i < pixelated_image.width() / pix_size + 1; i++) {
            for (int j = 0; j < pixelated_image.height() / pix_size + 1; j++) {
                int avg_r  = 0;
                int avg_g  = 0;
                int avg_b  = 0;
                int pixels = 0;

                for (int x = i * pix_size; x < (i + 1) * pix_size && x < pixelated_image.width(); x++) {
                    for (int y = j * pix_size; y < (j + 1) * pix_size && y < pixelated_image.height(); y++) {
                        int pixel = pixelated_image.pixel(x, y);

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

                    for (int x = i * pix_size; x < (i + 1) * pix_size && x < pixelated_image.width(); x++) {
                        for (int y = j * pix_size; y < (j + 1) * pix_size && y < pixelated_image.height(); y++) {
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
