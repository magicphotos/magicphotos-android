#include <qmath.h>
#include <QFileInfo>
#include <QThread>
#include <QImageReader>
#include <QPainter>

#include "sketcheditor.h"

SketchEditor::SketchEditor(QDeclarativeItem *parent) : QDeclarativeItem(parent)
{
    IsChanged      = false;
    CurrentMode    = ModeScroll;
    HelperSize     = 0;
    GaussianRadius = 0;

    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);

    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);
    setFlag(QGraphicsItem::ItemHasNoContents,           false);
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

int SketchEditor::helperSize() const
{
    return HelperSize;
}

void SketchEditor::setHelperSize(const int &size)
{
    HelperSize = size;
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

void SketchEditor::openImage(const QString &image_url)
{
    QString image_file = QUrl(image_url).toLocalFile();

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

void SketchEditor::saveImage(const QString &image_url)
{
    QString file_name = QUrl(image_url).toLocalFile();

    if (!file_name.isNull()) {
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
    } else {
        emit imageSaveFailed();
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

        update();
    }
}

void SketchEditor::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget*)
{
    qreal scale = 1.0;

    if (CurrentImage.width() != 0 && CurrentImage.height() != 0) {
        scale = width() / CurrentImage.width() < height() / CurrentImage.height() ?
                width() / CurrentImage.width() : height() / CurrentImage.height();
    }

    bool antialiasing = painter->testRenderHint(QPainter::Antialiasing);

    if (smooth()) {
        painter->setRenderHint(QPainter::Antialiasing, true);
    }

    QRectF src_rect(option->exposedRect.left()   / scale,
                    option->exposedRect.top()    / scale,
                    option->exposedRect.width()  / scale,
                    option->exposedRect.height() / scale);

    painter->drawImage(option->exposedRect, CurrentImage.copy(src_rect.toRect()));

    painter->setRenderHint(QPainter::Antialiasing, antialiasing);
}

void SketchEditor::effectedImageReady(const QImage &effected_image)
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

void SketchEditor::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        ChangeImageAt(true, event->pos().x(), event->pos().y());

        emit mouseEvent(MousePressed, event->pos().x(), event->pos().y());
    }
}

void SketchEditor::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        ChangeImageAt(false, event->pos().x(), event->pos().y());

        emit mouseEvent(MouseMoved, event->pos().x(), event->pos().y());
    }
}

void SketchEditor::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        emit mouseEvent(MouseReleased, event->pos().x(), event->pos().y());
    }
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

void SketchEditor::ChangeImageAt(bool save_undo, int center_x, int center_y)
{
    if (CurrentMode != ModeScroll) {
        if (save_undo) {
            SaveUndoImage();
        }

        qreal scale = 1.0;

        if (CurrentImage.width() != 0 && CurrentImage.height() != 0) {
            scale = width() / CurrentImage.width() < height() / CurrentImage.height() ?
                    width() / CurrentImage.width() : height() / CurrentImage.height();
        }

        int img_center_x = center_x   / scale;
        int img_center_y = center_y   / scale;
        int radius       = BRUSH_SIZE / scale;

        for (int x = img_center_x - radius; x <= img_center_x + radius; x++) {
            for (int y = img_center_y - radius; y <= img_center_y + radius; y++) {
                if (x >= 0 && x < CurrentImage.width() && y >= 0 && y < CurrentImage.height() && qSqrt(qPow(x - img_center_x, 2) + qPow(y - img_center_y, 2)) <= radius) {
                    if (CurrentMode == ModeOriginal) {
                        CurrentImage.setPixel(x, y, OriginalImage.pixel(x, y));
                    } else {
                        CurrentImage.setPixel(x, y, EffectedImage.pixel(x, y));
                    }
                }
            }
        }

        IsChanged = true;

        update(center_x - BRUSH_SIZE, center_y - BRUSH_SIZE, BRUSH_SIZE * 2, BRUSH_SIZE * 2);

        QImage helper_image = CurrentImage.copy(img_center_x - (HelperSize / scale) / 2,
                                                img_center_y - (HelperSize / scale) / 2,
                                                HelperSize / scale,
                                                HelperSize / scale).scaledToWidth(HelperSize);

        emit helperImageReady(helper_image);
    }
}

SketchPreviewGenerator::SketchPreviewGenerator(QDeclarativeItem *parent) : QDeclarativeItem(parent)
{
    SketchGeneratorRunning = false;
    RestartSketchGenerator = false;
    GaussianRadius         = 0;

    setFlag(QGraphicsItem::ItemHasNoContents, false);
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
        if (SketchGeneratorRunning) {
            RestartSketchGenerator = true;
        } else {
            StartSketchGenerator();
        }
    }
}

void SketchPreviewGenerator::openImage(const QString &image_url)
{
    QString image_file = QUrl(image_url).toLocalFile();

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
                LoadedImage = LoadedImage.convertToFormat(QImage::Format_RGB16);

                if (!LoadedImage.isNull()) {
                    emit imageOpened();

                    if (SketchGeneratorRunning) {
                        RestartSketchGenerator = true;
                    } else {
                        StartSketchGenerator();
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

void SketchPreviewGenerator::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    qreal scale = 1.0;

    if (SketchImage.width() != 0 && SketchImage.height() != 0) {
        scale = width() / SketchImage.width() < height() / SketchImage.height() ?
                width() / SketchImage.width() : height() / SketchImage.height();
    }

    bool antialiasing = painter->testRenderHint(QPainter::Antialiasing);

    if (smooth()) {
        painter->setRenderHint(QPainter::Antialiasing, true);
    }

    QRectF src_rect(0, 0,
                    SketchImage.width(),
                    SketchImage.height());
    QRectF dst_rect((width()  - SketchImage.width()  * scale) / 2,
                    (height() - SketchImage.height() * scale) / 2,
                    SketchImage.width()  * scale,
                    SketchImage.height() * scale);

    painter->drawImage(dst_rect, SketchImage, src_rect);

    painter->setRenderHint(QPainter::Antialiasing, antialiasing);
}

void SketchPreviewGenerator::sketchImageReady(const QImage &sketch_image)
{
    SketchGeneratorRunning = false;
    SketchImage            = sketch_image;

    setImplicitWidth(SketchImage.width());
    setImplicitHeight(SketchImage.height());

    update();

    emit generationFinished();

    if (RestartSketchGenerator) {
        StartSketchGenerator();

        RestartSketchGenerator = false;
    }
}

void SketchPreviewGenerator::StartSketchGenerator()
{
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

    thread->start(QThread::LowPriority);

    SketchGeneratorRunning = true;

    emit generationStarted();
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
