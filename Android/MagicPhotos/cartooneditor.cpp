#include <QtCore/qmath.h>
#include <QtCore/QFileInfo>
#include <QtCore/QThread>
#include <QtGui/QImageReader>
#include <QtGui/QPainter>

#include "cartooneditor.h"

CartoonEditor::CartoonEditor(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    IsChanged        = false;
    CurrentMode      = ModeScroll;
    HelperSize       = 0;
    GaussianRadius   = 0;
    CartoonThreshold = 0;

    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);

    setRenderTarget(QQuickPaintedItem::FramebufferObject);

    setFlag(QQuickItem::ItemHasContents, true);
}

CartoonEditor::~CartoonEditor()
{
}

int CartoonEditor::mode() const
{
    return CurrentMode;
}

void CartoonEditor::setMode(const int &mode)
{
    CurrentMode = mode;
}

int CartoonEditor::helperSize() const
{
    return HelperSize;
}

void CartoonEditor::setHelperSize(const int &size)
{
    HelperSize = size;
}

int CartoonEditor::radius() const
{
    return GaussianRadius;
}

void CartoonEditor::setRadius(const int &radius)
{
    GaussianRadius = radius;
}

int CartoonEditor::threshold() const
{
    return CartoonThreshold;
}

void CartoonEditor::setThreshold(const int &threshold)
{
    CartoonThreshold = threshold;
}

bool CartoonEditor::changed() const
{
    return IsChanged;
}

void CartoonEditor::openImage(const QString &image_file)
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
                LoadedImage = LoadedImage.convertToFormat(QImage::Format_RGB16);

                if (!LoadedImage.isNull()) {
                    QThread               *thread    = new QThread();
                    CartoonImageGenerator *generator = new CartoonImageGenerator();

                    generator->moveToThread(thread);

                    QObject::connect(thread,    SIGNAL(started()),                  generator, SLOT(start()));
                    QObject::connect(thread,    SIGNAL(finished()),                 thread,    SLOT(deleteLater()));
                    QObject::connect(generator, SIGNAL(imageReady(const QImage &)), this,      SLOT(effectedImageReady(const QImage &)));
                    QObject::connect(generator, SIGNAL(finished()),                 thread,    SLOT(quit()));
                    QObject::connect(generator, SIGNAL(finished()),                 generator, SLOT(deleteLater()));

                    generator->setGaussianRadius(GaussianRadius);
                    generator->setCartoonThreshold(CartoonThreshold);
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

void CartoonEditor::saveImage(const QString &image_file)
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

void CartoonEditor::undo()
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

void CartoonEditor::paint(QPainter *painter)
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

void CartoonEditor::effectedImageReady(const QImage &effected_image)
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

void CartoonEditor::mousePressEvent(QMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        ChangeImageAt(true, event->pos().x(), event->pos().y());

        emit mouseEvent(MousePressed, event->pos().x(), event->pos().y());
    }
}

void CartoonEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        ChangeImageAt(false, event->pos().x(), event->pos().y());

        emit mouseEvent(MouseMoved, event->pos().x(), event->pos().y());
    }
}

void CartoonEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        emit mouseEvent(MouseReleased, event->pos().x(), event->pos().y());
    }
}

void CartoonEditor::SaveUndoImage()
{
    UndoStack.push(CurrentImage);

    if (UndoStack.size() > UNDO_DEPTH) {
        for (int i = 0; i < UndoStack.size() - UNDO_DEPTH; i++) {
            UndoStack.remove(0);
        }
    }

    emit undoAvailabilityChanged(true);
}

void CartoonEditor::ChangeImageAt(bool save_undo, int center_x, int center_y)
{
    if (CurrentMode != ModeScroll) {
        if (save_undo) {
            SaveUndoImage();
        }

        int radius = BRUSH_SIZE / scale();

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

CartoonPreviewGenerator::CartoonPreviewGenerator(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    CartoonGeneratorRunning = false;
    RestartCartoonGenerator = false;
    GaussianRadius          = 0;
    CartoonThreshold        = 0;

    setRenderTarget(QQuickPaintedItem::FramebufferObject);

    setFlag(QQuickItem::ItemHasContents, true);
}

CartoonPreviewGenerator::~CartoonPreviewGenerator()
{
}

int CartoonPreviewGenerator::radius() const
{
    return GaussianRadius;
}

void CartoonPreviewGenerator::setRadius(const int &radius)
{
    GaussianRadius = radius;

    if (!LoadedImage.isNull()) {
        if (CartoonGeneratorRunning) {
            RestartCartoonGenerator = true;
        } else {
            StartCartoonGenerator();
        }
    }
}

int CartoonPreviewGenerator::threshold() const
{
    return CartoonThreshold;
}

void CartoonPreviewGenerator::setThreshold(const int &threshold)
{
    CartoonThreshold = threshold;

    if (!LoadedImage.isNull()) {
        if (CartoonGeneratorRunning) {
            RestartCartoonGenerator = true;
        } else {
            StartCartoonGenerator();
        }
    }
}

void CartoonPreviewGenerator::openImage(const QString &image_file)
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
                LoadedImage = LoadedImage.convertToFormat(QImage::Format_RGB16);

                if (!LoadedImage.isNull()) {
                    emit imageOpened();

                    if (CartoonGeneratorRunning) {
                        RestartCartoonGenerator = true;
                    } else {
                        StartCartoonGenerator();
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

void CartoonPreviewGenerator::paint(QPainter *painter)
{
    bool smooth_pixmap = painter->testRenderHint(QPainter::SmoothPixmapTransform);

    if (smooth()) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    } else {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
    }

    if (!CartoonImage.isNull()) {
        QImage image = CartoonImage.scaled(QSize(contentsBoundingRect().width(),
                                                 contentsBoundingRect().height()),
                                           Qt::KeepAspectRatio,
                                           smooth() ? Qt::SmoothTransformation : Qt::FastTransformation);

        painter->drawPixmap(QPoint((contentsBoundingRect().width()  - image.width())  / 2,
                                   (contentsBoundingRect().height() - image.height()) / 2), QPixmap::fromImage(image));
    }

    painter->setRenderHint(QPainter::SmoothPixmapTransform, smooth_pixmap);
}

void CartoonPreviewGenerator::cartoonImageReady(const QImage &cartoon_image)
{
    CartoonGeneratorRunning = false;
    CartoonImage            = cartoon_image;

    setImplicitWidth(CartoonImage.width());
    setImplicitHeight(CartoonImage.height());

    update();

    emit generationFinished();

    if (RestartCartoonGenerator) {
        StartCartoonGenerator();

        RestartCartoonGenerator = false;
    }
}

void CartoonPreviewGenerator::StartCartoonGenerator()
{
    QThread               *thread    = new QThread();
    CartoonImageGenerator *generator = new CartoonImageGenerator();

    generator->moveToThread(thread);

    QObject::connect(thread,    SIGNAL(started()),                  generator, SLOT(start()));
    QObject::connect(thread,    SIGNAL(finished()),                 thread,    SLOT(deleteLater()));
    QObject::connect(generator, SIGNAL(imageReady(const QImage &)), this,      SLOT(cartoonImageReady(const QImage &)));
    QObject::connect(generator, SIGNAL(finished()),                 thread,    SLOT(quit()));
    QObject::connect(generator, SIGNAL(finished()),                 generator, SLOT(deleteLater()));

    generator->setGaussianRadius(GaussianRadius);
    generator->setCartoonThreshold(CartoonThreshold);
    generator->setInput(LoadedImage);

    thread->start(QThread::LowPriority);

    CartoonGeneratorRunning = true;

    emit generationStarted();
}

CartoonImageGenerator::CartoonImageGenerator(QObject *parent) : QObject(parent)
{
    GaussianRadius   = 0;
    CartoonThreshold = 0;
}

CartoonImageGenerator::~CartoonImageGenerator()
{
}

void CartoonImageGenerator::setGaussianRadius(const int &radius)
{
    GaussianRadius = radius;
}

void CartoonImageGenerator::setCartoonThreshold(const int &threshold)
{
    CartoonThreshold = threshold;
}

void CartoonImageGenerator::setInput(const QImage &input_image)
{
    InputImage = input_image;
}

void CartoonImageGenerator::start()
{
    QImage blur_image    = InputImage;
    QImage cartoon_image = InputImage;

    // Make Gaussian blur of original image, if applicable

    if (GaussianRadius != 0) {
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
                    p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
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
                    p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
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
                    p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
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
                    p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
                }
            }
        }

        blur_image = blur_image.convertToFormat(format);
    }

    // Apply Cartoon filter

    QVector<int> src_buf(blur_image.width() * blur_image.height() * 4, 0);
    QVector<int> dst_buf(blur_image.width() * blur_image.height() * 4, 0);

    for (int y = 0; y < blur_image.height(); y++) {
        for (int x = 0; x < blur_image.width(); x++) {
            QRgb color = blur_image.pixel(x, y);

            src_buf[(y * blur_image.width() + x) * 4]     = qBlue(color);
            src_buf[(y * blur_image.width() + x) * 4 + 1] = qGreen(color);
            src_buf[(y * blur_image.width() + x) * 4 + 2] = qRed(color);
            src_buf[(y * blur_image.width() + x) * 4 + 3] = qAlpha(color);
        }
    }

    int  offset;
    int  blue_g, green_g, red_g;
    int  blue, green, red;
    bool exceeds_threshold;

    for (int y = 1; y < blur_image.height() - 1; y++)
    {
        for (int x = 1; x < blur_image.width() - 1; x++)
        {
            offset = y * blur_image.width() * 4 + x * 4;

            blue_g  = abs(src_buf[offset - 4]                      - src_buf[offset + 4]);
            blue_g += abs(src_buf[offset - blur_image.width() * 4] - src_buf[offset + blur_image.width() * 4]);

            offset++;

            green_g  = abs(src_buf[offset - 4]                      - src_buf[offset + 4]);
            green_g += abs(src_buf[offset - blur_image.width() * 4] - src_buf[offset + blur_image.width() * 4]);

            offset++;

            red_g  = abs(src_buf[offset - 4]                      - src_buf[offset + 4]);
            red_g += abs(src_buf[offset - blur_image.width() * 4] - src_buf[offset + blur_image.width() * 4]);

            if (blue_g + green_g + red_g > CartoonThreshold)
            {
                exceeds_threshold = true;
            }
            else
            {
                offset -= 2;

                blue_g = abs(src_buf[offset - 4] - src_buf[offset + 4]);

                offset++;

                green_g = abs(src_buf[offset - 4] - src_buf[offset + 4]);

                offset++;

                red_g = abs(src_buf[offset - 4] - src_buf[offset + 4]);

                if (blue_g + green_g + red_g > CartoonThreshold)
                {
                    exceeds_threshold = true;
                }
                else
                {
                    offset -= 2;

                    blue_g = abs(src_buf[offset - blur_image.width() * 4] - src_buf[offset + blur_image.width() * 4]);

                    offset++;

                    green_g = abs(src_buf[offset - blur_image.width() * 4] - src_buf[offset + blur_image.width() * 4]);

                    offset++;

                    red_g = abs(src_buf[offset - blur_image.width() * 4] - src_buf[offset + blur_image.width() * 4]);

                    if (blue_g + green_g + red_g > CartoonThreshold)
                    {
                        exceeds_threshold = true;
                    }
                    else
                    {
                        offset -= 2;

                        blue_g  = abs(src_buf[offset - 4 - blur_image.width() * 4] - src_buf[offset + 4 + blur_image.width() * 4]);
                        blue_g += abs(src_buf[offset + 4 - blur_image.width() * 4] - src_buf[offset - 4 + blur_image.width() * 4]);

                        offset++;

                        green_g  = abs(src_buf[offset - 4 - blur_image.width() * 4] - src_buf[offset + 4 + blur_image.width() * 4]);
                        green_g += abs(src_buf[offset + 4 - blur_image.width() * 4] - src_buf[offset - 4 + blur_image.width() * 4]);

                        offset++;

                        red_g  = abs(src_buf[offset - 4 - blur_image.width() * 4] - src_buf[offset + 4 + blur_image.width() * 4]);
                        red_g += abs(src_buf[offset + 4 - blur_image.width() * 4] - src_buf[offset - 4 + blur_image.width() * 4]);

                        if (blue_g + green_g + red_g > CartoonThreshold)
                        {
                            exceeds_threshold = true;
                        }
                        else
                        {
                            exceeds_threshold = false;
                        }
                    }
                }
            }

            offset -= 2;

            if (exceeds_threshold)
            {
                blue  = 0;
                green = 0;
                red   = 0;
            }
            else
            {
                blue  = src_buf[offset];
                green = src_buf[offset + 1];
                red   = src_buf[offset + 2];
            }

            blue  = (blue  > 255 ? 255 : (blue  < 0 ? 0 : blue));
            green = (green > 255 ? 255 : (green < 0 ? 0 : green));
            red   = (red   > 255 ? 255 : (red   < 0 ? 0 : red));

            dst_buf[offset]     = blue;
            dst_buf[offset + 1] = green;
            dst_buf[offset + 2] = red;
            dst_buf[offset + 3] = src_buf[offset + 3];
        }
    }

    for (int y = 0; y < cartoon_image.height(); y++) {
        for (int x = 0; x < cartoon_image.width(); x++) {
            cartoon_image.setPixel(x, y, qRgba(dst_buf[(y * cartoon_image.width() + x) * 4 + 2], dst_buf[(y * cartoon_image.width() + x) * 4 + 1],
                                               dst_buf[(y * cartoon_image.width() + x) * 4],     dst_buf[(y * cartoon_image.width() + x) * 4 + 3]));
        }
    }

    emit imageReady(cartoon_image);
    emit finished();
}
