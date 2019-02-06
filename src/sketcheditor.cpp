#include <QtCore/QtMath>
#include <QtCore/QFileInfo>
#include <QtCore/QThread>
#include <QtGui/QTransform>
#include <QtGui/QImageReader>
#include <QtGui/QPainter>

#include "sketcheditor.h"

SketchEditor::SketchEditor(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    IsChanged      = false;
    CurrentMode    = ModeScroll;
    BrushSize      = 0;
    HelperSize     = 0;
    GaussianRadius = 0;
    BrushOpacity   = 0.0;

    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);

    setFlag(QQuickItem::ItemHasContents, true);

    QObject::connect(this, &SketchEditor::scaleChanged, this, &SketchEditor::scaleWasChanged);
}

SketchEditor::~SketchEditor()
{
}

int SketchEditor::mode() const
{
    return CurrentMode;
}

void SketchEditor::setMode(int mode)
{
    CurrentMode = mode;
}

int SketchEditor::brushSize() const
{
    return BrushSize;
}

void SketchEditor::setBrushSize(int size)
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

int SketchEditor::helperSize() const
{
    return HelperSize;
}

void SketchEditor::setHelperSize(int size)
{
    HelperSize = size;
}

int SketchEditor::radius() const
{
    return GaussianRadius;
}

void SketchEditor::setRadius(int radius)
{
    GaussianRadius = radius;
}

qreal SketchEditor::brushOpacity() const
{
    return BrushOpacity;
}

void SketchEditor::setBrushOpacity(qreal opacity)
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

bool SketchEditor::changed() const
{
    return IsChanged;
}

void SketchEditor::openImage(QString image_file, int image_orientation)
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

                LoadedImage = LoadedImage.convertToFormat(QImage::Format_RGB16);

                if (!LoadedImage.isNull()) {
                    QThread              *thread    = new QThread();
                    SketchImageGenerator *generator = new SketchImageGenerator();

                    generator->moveToThread(thread);

                    QObject::connect(thread,    &QThread::started,                 generator, &SketchImageGenerator::start);
                    QObject::connect(thread,    &QThread::finished,                thread,    &QThread::deleteLater);
                    QObject::connect(generator, &SketchImageGenerator::imageReady, this,      &SketchEditor::effectedImageReady);
                    QObject::connect(generator, &SketchImageGenerator::finished,   thread,    &QThread::quit);
                    QObject::connect(generator, &SketchImageGenerator::finished,   generator, &SketchImageGenerator::deleteLater);

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

void SketchEditor::saveImage(QString image_file)
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

void SketchEditor::paint(QPainter *painter)
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

void SketchEditor::effectedImageReady(QImage effected_image)
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

void SketchEditor::scaleWasChanged()
{
    int brush_width = qMax(1, qMin(qMin(qFloor(BrushSize / scale()) * 2, CurrentImage.width()), CurrentImage.height()));

    BrushImage = BrushTemplateImage.scaledToWidth(brush_width);
}

void SketchEditor::mousePressEvent(QMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        ChangeImageAt(true, event->pos().x(), event->pos().y());

        emit mouseEvent(MousePressed, event->pos().x(), event->pos().y());
    }
}

void SketchEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        ChangeImageAt(false, event->pos().x(), event->pos().y());

        emit mouseEvent(MouseMoved, event->pos().x(), event->pos().y());
    }
}

void SketchEditor::mouseReleaseEvent(QMouseEvent *event)
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

SketchPreviewGenerator::SketchPreviewGenerator(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    SketchGeneratorRunning = false;
    RestartSketchGenerator = false;
    GaussianRadius         = 0;

    setFlag(QQuickItem::ItemHasContents, true);
}

SketchPreviewGenerator::~SketchPreviewGenerator()
{
}

int SketchPreviewGenerator::radius() const
{
    return GaussianRadius;
}

void SketchPreviewGenerator::setRadius(int radius)
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

void SketchPreviewGenerator::openImage(QString image_file, int image_orientation)
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

void SketchPreviewGenerator::paint(QPainter *painter)
{
    bool smooth_pixmap = painter->testRenderHint(QPainter::SmoothPixmapTransform);

    if (smooth()) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    } else {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
    }

    if (!SketchImage.isNull()) {
        QImage image = SketchImage.scaled(QSize(qFloor(contentsBoundingRect().width()),
                                                qFloor(contentsBoundingRect().height())),
                                          Qt::KeepAspectRatio,
                                          smooth() ? Qt::SmoothTransformation : Qt::FastTransformation);

        painter->drawPixmap(QPoint(qFloor((contentsBoundingRect().width()  - image.width())  / 2),
                                   qFloor((contentsBoundingRect().height() - image.height()) / 2)), QPixmap::fromImage(image));
    }

    painter->setRenderHint(QPainter::SmoothPixmapTransform, smooth_pixmap);
}

void SketchPreviewGenerator::sketchImageReady(QImage sketch_image)
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

    QObject::connect(thread,    &QThread::started,                 generator, &SketchImageGenerator::start);
    QObject::connect(thread,    &QThread::finished,                thread,    &QThread::deleteLater);
    QObject::connect(generator, &SketchImageGenerator::imageReady, this,      &SketchPreviewGenerator::sketchImageReady);
    QObject::connect(generator, &SketchImageGenerator::finished,   thread,    &QThread::quit);
    QObject::connect(generator, &SketchImageGenerator::finished,   generator, &SketchImageGenerator::deleteLater);

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

void SketchImageGenerator::setGaussianRadius(int radius)
{
    GaussianRadius = radius;
}

void SketchImageGenerator::setInput(QImage input_image)
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
