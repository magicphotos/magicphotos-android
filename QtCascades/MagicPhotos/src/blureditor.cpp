#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QThread>
#include <QtGui/QTransform>
#include <QtGui/QImageReader>
#include <QtGui/QPainter>

#include <bb/ImageData>
#include <bb/cascades/Image>

#include <libexif/exif-loader.h>
#include <libexif/exif-data.h>
#include <libexif/exif-entry.h>

#include "blureditor.h"

BlurEditor::BlurEditor() : bb::cascades::CustomControl()
{
    IsChanged       = false;
    CurrentMode     = ModeScroll;
    BrushSize       = 0;
    HelperSize      = 0;
    GaussianRadius  = 0;
    BrushOpacity    = 0.0;
    Scale           = 1.0;
    ResolutionLimit = 0.0;
    Helper          = NULL;
}

BlurEditor::~BlurEditor()
{
}

int BlurEditor::mode() const
{
    return CurrentMode;
}

void BlurEditor::setMode(const int &mode)
{
    CurrentMode = mode;
}

int BlurEditor::brushSize() const
{
    return BrushSize;
}

void BlurEditor::setBrushSize(const int &size)
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
                    BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, (int)(0xFF * (BrushSize - r) / (BrushSize * (1.0 - BrushOpacity)))));
                }
            } else {
                BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, 0x00));
            }
        }
    }

    int brush_width = qMax(1, qMin(qMin((int)(BrushSize / Scale) * 2, CurrentImage.width()), CurrentImage.height()));

    BrushImage = BrushTemplateImage.scaledToWidth(brush_width);
}

int BlurEditor::helperSize() const
{
    return HelperSize;
}

void BlurEditor::setHelperSize(const int &size)
{
    HelperSize = size;
}

int BlurEditor::radius() const
{
    return GaussianRadius;
}

void BlurEditor::setRadius(const int &radius)
{
    GaussianRadius = radius;
}

qreal BlurEditor::brushOpacity() const
{
    return BrushOpacity;
}

void BlurEditor::setBrushOpacity(const qreal &opacity)
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
                    BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, (int)(0xFF * (BrushSize - r) / (BrushSize * (1.0 - BrushOpacity)))));
                }
            } else {
                BrushTemplateImage.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, 0x00));
            }
        }
    }

    int brush_width = qMax(1, qMin(qMin((int)(BrushSize / Scale) * 2, CurrentImage.width()), CurrentImage.height()));

    BrushImage = BrushTemplateImage.scaledToWidth(brush_width);
}

qreal BlurEditor::scale() const
{
    return Scale;
}

void BlurEditor::setScale(const qreal &scale)
{
    Scale = scale;

    int brush_width = qMax(1, qMin(qMin((int)(BrushSize / Scale) * 2, CurrentImage.width()), CurrentImage.height()));

    BrushImage = BrushTemplateImage.scaledToWidth(brush_width);
}

qreal BlurEditor::resolutionLimit() const
{
    return ResolutionLimit;
}

void BlurEditor::setResolutionLimit(const qreal &limit)
{
    ResolutionLimit = limit;
}

bb::cascades::ImageView *BlurEditor::helper() const
{
    return Helper;
}

void BlurEditor::setHelper(bb::cascades::ImageView *helper)
{
    Helper = helper;
}

bool BlurEditor::changed() const
{
    return IsChanged;
}

void BlurEditor::openImage(const QString &image_file)
{
    int         img_orientation = 0;
    ExifLoader *exif_loader     = exif_loader_new();

    if (exif_loader != NULL) {
        ExifData *exif_data;

        exif_loader_write_file(exif_loader, image_file.toUtf8().data());

        exif_data = exif_loader_get_data(exif_loader);

        if (exif_data != NULL) {
            ExifByteOrder exif_bo    = exif_data_get_byte_order(exif_data);
            ExifEntry    *exif_entry = exif_content_get_entry(exif_data->ifd[EXIF_IFD_0], EXIF_TAG_ORIENTATION);

            if (exif_entry != NULL) {
                img_orientation = exif_get_short(exif_entry->data, exif_bo);

                exif_entry_unref(exif_entry);
            }

            exif_data_unref(exif_data);
        }

        exif_loader_unref(exif_loader);
    }

    QImageReader reader(image_file);

    if (reader.canRead()) {
        QSize size = reader.size();

        if (ResolutionLimit > 0.1 && size.width() * size.height() > ResolutionLimit * 1000000.0) {
            qreal factor = qSqrt((size.width() * size.height()) / (ResolutionLimit * 1000000.0));

            size.setWidth(size.width()   / factor);
            size.setHeight(size.height() / factor);

            reader.setScaledSize(size);
        }

        LoadedImage = reader.read();

        if (!LoadedImage.isNull()) {
            if (img_orientation == 3) {
                QTransform transform;

                transform.rotate(180);

                LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.width(), LoadedImage.height());
            } else if (img_orientation == 6) {
                QTransform transform;

                transform.rotate(90);

                LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
            } else if (img_orientation == 8) {
                QTransform transform;

                transform.rotate(270);

                LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
            }

            LoadedImage = LoadedImage.convertToFormat(QImage::Format_RGB16);

            if (!LoadedImage.isNull()) {
                QThread            *thread    = new QThread();
                BlurImageGenerator *generator = new BlurImageGenerator();

                generator->moveToThread(thread);

                QObject::connect(thread,    SIGNAL(started()),                  generator, SLOT(start()));
                QObject::connect(thread,    SIGNAL(finished()),                 thread,    SLOT(deleteLater()));
                QObject::connect(generator, SIGNAL(imageReady(const QImage &)), this,      SLOT(effectedImageReady(const QImage &)));
                QObject::connect(generator, SIGNAL(finished()),                 thread,    SLOT(quit()));
                QObject::connect(generator, SIGNAL(finished()),                 generator, SLOT(deleteLater()));

                generator->setGaussianRadius(GaussianRadius);
                generator->setInput(LoadedImage);

                thread->start();
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
}

void BlurEditor::changeImageAt(bool save_undo, int center_x, int center_y)
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

        RepaintImage(false, QRect(img_x, img_y, width, height));
        RepaintHelper(center_x, center_y);
    }
}

void BlurEditor::undo()
{
    if (UndoStack.size() > 0) {
        CurrentImage = UndoStack.pop();

        if (UndoStack.size() == 0) {
            emit undoAvailabilityChanged(false);
        }

        IsChanged = true;

        RepaintImage(true);
    }
}

void BlurEditor::addFragment(int x, int y, bb::cascades::ImageView *fragment)
{
    FragmentsMap[QPair<int, int>(x, y)] = fragment;
}

void BlurEditor::delFragment(int x, int y)
{
    FragmentsMap.remove(QPair<int, int>(x, y));
}

QList<QObject*> BlurEditor::getFragments()
{
    QList<QObject*> result;

    for (int i = 0; i < FragmentsMap.keys().size(); i++) {
        result.append(FragmentsMap[FragmentsMap.keys().at(i)]);
    }

    return result;
}

void BlurEditor::effectedImageReady(const QImage &effected_image)
{
    OriginalImage = LoadedImage;
    EffectedImage = effected_image;
    CurrentImage  = EffectedImage;

    LoadedImage = QImage();

    UndoStack.clear();

    IsChanged = true;

    emit prepareFragments(FRAGMENT_SIZE, CurrentImage.width(), CurrentImage.height());

    RepaintImage(true);

    int brush_width = qMax(1, qMin(qMin((int)(BrushSize / Scale) * 2, CurrentImage.width()), CurrentImage.height()));

    BrushImage = BrushTemplateImage.scaledToWidth(brush_width);

    emit undoAvailabilityChanged(false);
    emit imageOpened();
}

void BlurEditor::SaveUndoImage()
{
    UndoStack.push(CurrentImage);

    if (UndoStack.size() > UNDO_DEPTH) {
        for (int i = 0; i < UndoStack.size() - UNDO_DEPTH; i++) {
            UndoStack.remove(0);
        }
    }

    emit undoAvailabilityChanged(true);
}

void BlurEditor::RepaintImage(bool full, QRect rect)
{
    if (!CurrentImage.isNull()) {
        int fragment_size = FRAGMENT_SIZE;

        if (full) {
            rect.setX(0);
            rect.setY(0);
            rect.setWidth(CurrentImage.width());
            rect.setHeight(CurrentImage.height());
        } else {
            if (rect.x() >= CurrentImage.width()) {
                rect.setX(CurrentImage.width() - 1);
            }
            if (rect.y() >= CurrentImage.height()) {
                rect.setY(CurrentImage.height() - 1);
            }
            if (rect.x() < 0) {
                rect.setX(0);
            }
            if (rect.y() < 0) {
                rect.setY(0);
            }
            if (rect.x() + rect.width() > CurrentImage.width()) {
                rect.setWidth(CurrentImage.width() - rect.x());
            }
            if (rect.y() + rect.height() > CurrentImage.height()) {
                rect.setHeight(CurrentImage.height() - rect.y());
            }
        }

        for (int fx = fragment_size * (rect.x() / fragment_size); fx < rect.x() + rect.width();) {
            int fragment_width = qMax(0, qMin(fragment_size, CurrentImage.width() - fx));

            for (int fy = fragment_size * (rect.y() / fragment_size); fy < rect.y() + rect.height();) {
                int fragment_height = qMax(0, qMin(fragment_size, CurrentImage.height() - fy));

                bb::ImageData fragment_data = bb::ImageData(bb::PixelFormat::RGBA_Premultiplied, fragment_width, fragment_height);

                unsigned char *dst_line = fragment_data.pixels();

                for (int sy = fy; sy < fy + fragment_height; sy++) {
                    unsigned char *dst = dst_line;

                    for (int sx = fx; sx < fx + fragment_width; sx++) {
                        QRgb pixel = CurrentImage.pixel(sx, sy);

                        *dst++ = qRed(pixel);
                        *dst++ = qGreen(pixel);
                        *dst++ = qBlue(pixel);
                        *dst++ = qAlpha(pixel);
                    }

                    dst_line += fragment_data.bytesPerLine();
                }

                if (FragmentsMap.contains(QPair<int, int>(fx, fy))) {
                    FragmentsMap[QPair<int, int>(fx, fy)]->setImage(bb::cascades::Image(fragment_data));
                }

                fy = fy + fragment_height;
            }

            fx = fx + fragment_width;
        }
    }
}

void BlurEditor::RepaintHelper(int center_x, int center_y)
{
    if (!CurrentImage.isNull()) {
        QImage   helper_image = CurrentImage.copy(center_x - HelperSize / (Scale * 2),
                                                  center_y - HelperSize / (Scale * 2), HelperSize / Scale, HelperSize / Scale).scaledToWidth(HelperSize);
        QPainter painter(&helper_image);

        painter.setPen(QPen(Qt::white, 4, Qt::SolidLine));
        painter.drawPoint(helper_image.rect().center());

        bb::ImageData helper_image_data = bb::ImageData(bb::PixelFormat::RGBA_Premultiplied, helper_image.width(), helper_image.height());

        unsigned char *dst_line = helper_image_data.pixels();

        for (int y = 0; y < helper_image_data.height(); y++) {
            unsigned char *dst = dst_line;

            for (int x = 0; x < helper_image_data.width(); x++) {
                QRgb pixel = helper_image.pixel(x, y);

                *dst++ = qRed(pixel);
                *dst++ = qGreen(pixel);
                *dst++ = qBlue(pixel);
                *dst++ = qAlpha(pixel);
            }

            dst_line += helper_image_data.bytesPerLine();
        }

        if (Helper != NULL) {
            Helper->setImage(bb::cascades::Image(helper_image_data));
        }
    }
}

BlurPreviewGenerator::BlurPreviewGenerator() : bb::cascades::CustomControl()
{
    GaussianRadius = 0;
    Preview        = NULL;
}

BlurPreviewGenerator::~BlurPreviewGenerator()
{
}

int BlurPreviewGenerator::radius() const
{
    return GaussianRadius;
}

void BlurPreviewGenerator::setRadius(const int &radius)
{
    GaussianRadius = radius;

    if (!LoadedImage.isNull()) {
        QThread            *thread    = new QThread();
        BlurImageGenerator *generator = new BlurImageGenerator();

        generator->moveToThread(thread);

        QObject::connect(thread,    SIGNAL(started()),                  generator, SLOT(start()));
        QObject::connect(thread,    SIGNAL(finished()),                 thread,    SLOT(deleteLater()));
        QObject::connect(generator, SIGNAL(imageReady(const QImage &)), this,      SLOT(blurImageReady(const QImage &)));
        QObject::connect(generator, SIGNAL(finished()),                 thread,    SLOT(quit()));
        QObject::connect(generator, SIGNAL(finished()),                 generator, SLOT(deleteLater()));

        generator->setGaussianRadius(GaussianRadius);
        generator->setInput(LoadedImage);

        thread->start();

        emit generationStarted();
    }
}

bb::cascades::ImageView *BlurPreviewGenerator::preview() const
{
    return Preview;
}

void BlurPreviewGenerator::setPreview(bb::cascades::ImageView *preview)
{
    Preview = preview;
}

void BlurPreviewGenerator::openImage(const QString &image_file)
{
    int         img_orientation = 0;
    ExifLoader *exif_loader     = exif_loader_new();

    if (exif_loader != NULL) {
        ExifData *exif_data;

        exif_loader_write_file(exif_loader, image_file.toUtf8().data());

        exif_data = exif_loader_get_data(exif_loader);

        if (exif_data != NULL) {
            ExifByteOrder exif_bo    = exif_data_get_byte_order(exif_data);
            ExifEntry    *exif_entry = exif_content_get_entry(exif_data->ifd[EXIF_IFD_0], EXIF_TAG_ORIENTATION);

            if (exif_entry != NULL) {
                img_orientation = exif_get_short(exif_entry->data, exif_bo);

                exif_entry_unref(exif_entry);
            }

            exif_data_unref(exif_data);
        }

        exif_loader_unref(exif_loader);
    }

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
            if (img_orientation == 3) {
                QTransform transform;

                transform.rotate(180);

                LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.width(), LoadedImage.height());
            } else if (img_orientation == 6) {
                QTransform transform;

                transform.rotate(90);

                LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
            } else if (img_orientation == 8) {
                QTransform transform;

                transform.rotate(270);

                LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
            }

            LoadedImage = LoadedImage.convertToFormat(QImage::Format_RGB16);

            if (!LoadedImage.isNull()) {
                QThread            *thread    = new QThread();
                BlurImageGenerator *generator = new BlurImageGenerator();

                generator->moveToThread(thread);

                QObject::connect(thread,    SIGNAL(started()),                  generator, SLOT(start()));
                QObject::connect(thread,    SIGNAL(finished()),                 thread,    SLOT(deleteLater()));
                QObject::connect(generator, SIGNAL(imageReady(const QImage &)), this,      SLOT(blurImageReady(const QImage &)));
                QObject::connect(generator, SIGNAL(finished()),                 thread,    SLOT(quit()));
                QObject::connect(generator, SIGNAL(finished()),                 generator, SLOT(deleteLater()));

                generator->setGaussianRadius(GaussianRadius);
                generator->setInput(LoadedImage);

                thread->start();

                emit imageOpened();
                emit generationStarted();
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

void BlurPreviewGenerator::blurImageReady(const QImage &blur_image)
{
    BlurImage = blur_image;

    Repaint();

    emit generationFinished();
}

void BlurPreviewGenerator::Repaint()
{
    if (!BlurImage.isNull()) {
        bb::ImageData image_data = bb::ImageData(bb::PixelFormat::RGBA_Premultiplied, BlurImage.width(), BlurImage.height());

        unsigned char *dst_line = image_data.pixels();

        for (int y = 0; y < image_data.height(); y++) {
            unsigned char *dst = dst_line;

            for (int x = 0; x < image_data.width(); x++) {
                QRgb pixel = BlurImage.pixel(x, y);

                *dst++ = qRed(pixel);
                *dst++ = qGreen(pixel);
                *dst++ = qBlue(pixel);
                *dst++ = qAlpha(pixel);
            }

            dst_line += image_data.bytesPerLine();
        }

        if (Preview != NULL) {
            Preview->setImage(bb::cascades::Image(image_data));
        }
    }
}

BlurImageGenerator::BlurImageGenerator(QObject *parent) : QObject(parent)
{
    GaussianRadius = 0;
}

BlurImageGenerator::~BlurImageGenerator()
{
}

void BlurImageGenerator::setGaussianRadius(const int &radius)
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

    emit imageReady(blur_image);
    emit finished();
}
