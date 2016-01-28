#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtGui/QTransform>
#include <QtGui/QImageReader>
#include <QtGui/QPainter>

#include <bb/ImageData>
#include <bb/cascades/Image>

#include <libexif/exif-loader.h>
#include <libexif/exif-data.h>
#include <libexif/exif-entry.h>

#include "retoucheditor.h"

RetouchEditor::RetouchEditor() : bb::cascades::CustomControl()
{
    IsChanged            = false;
    IsSamplingPointValid = false;
    IsLastBlurPointValid = false;
    CurrentMode          = ModeScroll;
    BrushSize            = 0;
    HelperSize           = 0;
    BrushOpacity         = 0.0;
    Scale                = 1.0;
    ResolutionLimit      = 0.0;
    Helper               = NULL;
}

RetouchEditor::~RetouchEditor()
{
}

int RetouchEditor::mode() const
{
    return CurrentMode;
}

void RetouchEditor::setMode(const int &mode)
{
    CurrentMode = mode;
}

int RetouchEditor::brushSize() const
{
    return BrushSize;
}

void RetouchEditor::setBrushSize(const int &size)
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

int RetouchEditor::helperSize() const
{
    return HelperSize;
}

void RetouchEditor::setHelperSize(const int &size)
{
    HelperSize = size;
}

bool RetouchEditor::samplingPointValid() const
{
    return IsSamplingPointValid;
}

void RetouchEditor::setSamplingPointValid(const bool &valid)
{
    IsSamplingPointValid = valid;
}

bool RetouchEditor::lastBlurPointValid() const
{
    return IsLastBlurPointValid;
}

void RetouchEditor::setLastBlurPointValid(const bool &valid)
{
    IsLastBlurPointValid = valid;
}

int RetouchEditor::samplingPointX() const
{
    return SamplingPoint.x();
}

void RetouchEditor::setSamplingPointX(const int &x)
{
    SamplingPoint.setX(x);
}

int RetouchEditor::samplingPointY() const
{
    return SamplingPoint.y();
}

void RetouchEditor::setSamplingPointY(const int &y)
{
    SamplingPoint.setY(y);
}

int RetouchEditor::lastBlurPointX() const
{
    return LastBlurPoint.x();
}

void RetouchEditor::setLastBlurPointX(const int &x)
{
    LastBlurPoint.setX(x);
}

int RetouchEditor::lastBlurPointY() const
{
    return LastBlurPoint.y();
}

void RetouchEditor::setLastBlurPointY(const int &y)
{
    LastBlurPoint.setY(y);
}

qreal RetouchEditor::brushOpacity() const
{
    return BrushOpacity;
}

void RetouchEditor::setBrushOpacity(const qreal &opacity)
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

qreal RetouchEditor::scale() const
{
    return Scale;
}

void RetouchEditor::setScale(const qreal &scale)
{
    Scale = scale;

    int brush_width = qMax(1, qMin(qMin((int)(BrushSize / Scale) * 2, CurrentImage.width()), CurrentImage.height()));

    BrushImage = BrushTemplateImage.scaledToWidth(brush_width);
}

qreal RetouchEditor::resolutionLimit() const
{
    return ResolutionLimit;
}

void RetouchEditor::setResolutionLimit(const qreal &limit)
{
    ResolutionLimit = limit;
}

bb::cascades::ImageView *RetouchEditor::helper() const
{
    return Helper;
}

void RetouchEditor::setHelper(bb::cascades::ImageView *helper)
{
    Helper = helper;
}

bool RetouchEditor::changed() const
{
    return IsChanged;
}

int RetouchEditor::imageWidth() const
{
    return CurrentImage.width();
}

int RetouchEditor::imageHeight() const
{
    return CurrentImage.height();
}

void RetouchEditor::openImage(const QString &image_file)
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
                CurrentImage = LoadedImage;

                LoadedImage = QImage();

                UndoStack.clear();

                IsChanged = false;

                emit prepareFragments(FRAGMENT_SIZE, CurrentImage.width(), CurrentImage.height());

                RepaintImage(true);

                int brush_width = qMax(1, qMin(qMin((int)(BrushSize / Scale) * 2, CurrentImage.width()), CurrentImage.height()));

                BrushImage = BrushTemplateImage.scaledToWidth(brush_width);

                emit undoAvailabilityChanged(false);
                emit imageOpened();
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

void RetouchEditor::saveImage(const QString &image_file)
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

void RetouchEditor::changeImageAt(bool save_undo, int center_x, int center_y)
{
    if (CurrentMode == ModeClone || CurrentMode == ModeBlur) {
        if (save_undo) {
            SaveUndoImage();
        }

        int width  = qMin(BrushImage.width(),  CurrentImage.width());
        int height = qMin(BrushImage.height(), CurrentImage.height());

        int img_x = qMin(qMax(0, center_x - width  / 2), CurrentImage.width()  - width);
        int img_y = qMin(qMax(0, center_y - height / 2), CurrentImage.height() - height);

        if (CurrentMode == ModeClone) {
            int src_x = qMin(qMax(0, SamplingPoint.x() - width  / 2), CurrentImage.width()  - width);
            int src_y = qMin(qMax(0, SamplingPoint.y() - height / 2), CurrentImage.height() - height);

            QImage   brush_image(width, height, QImage::Format_ARGB32);
            QPainter brush_painter(&brush_image);

            brush_painter.setCompositionMode(QPainter::CompositionMode_Source);

            brush_painter.drawImage(QPoint(0, 0), CurrentImage, QRect(src_x, src_y, width, height));

            QPainter image_painter(&CurrentImage);

            brush_painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            brush_painter.drawImage(QPoint(0, 0), BrushImage);

            image_painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            image_painter.drawImage(QPoint(img_x, img_y), brush_image);
        } else if (CurrentMode == ModeBlur) {
            QRect  last_blur_rect(LastBlurPoint.x() - width / 2, LastBlurPoint.y() - height / 2, width, height);
            QImage last_blur_image;

            if (IsLastBlurPointValid) {
                if (last_blur_rect.x() >= CurrentImage.width()) {
                    last_blur_rect.setX(CurrentImage.width() - 1);
                }
                if (last_blur_rect.y() >= CurrentImage.height()) {
                    last_blur_rect.setY(CurrentImage.height() - 1);
                }
                if (last_blur_rect.x() < 0) {
                    last_blur_rect.setX(0);
                }
                if (last_blur_rect.y() < 0) {
                    last_blur_rect.setY(0);
                }
                if (last_blur_rect.x() + last_blur_rect.width() > CurrentImage.width()) {
                    last_blur_rect.setWidth(CurrentImage.width() - last_blur_rect.x());
                }
                if (last_blur_rect.y() + last_blur_rect.height() > CurrentImage.height()) {
                    last_blur_rect.setHeight(CurrentImage.height() - last_blur_rect.y());
                }

                last_blur_image = CurrentImage.copy(last_blur_rect);
            }

            QRect blur_rect(img_x, img_y, width, height);

            if (blur_rect.x() >= CurrentImage.width()) {
                blur_rect.setX(CurrentImage.width() - 1);
            }
            if (blur_rect.y() >= CurrentImage.height()) {
                blur_rect.setY(CurrentImage.height() - 1);
            }
            if (blur_rect.x() < 0) {
                blur_rect.setX(0);
            }
            if (blur_rect.y() < 0) {
                blur_rect.setY(0);
            }
            if (blur_rect.x() + blur_rect.width() > CurrentImage.width()) {
                blur_rect.setWidth(CurrentImage.width() - blur_rect.x());
            }
            if (blur_rect.y() + blur_rect.height() > CurrentImage.height()) {
                blur_rect.setHeight(CurrentImage.height() - blur_rect.y());
            }

            QImage blur_image = CurrentImage.copy(blur_rect).convertToFormat(QImage::Format_ARGB32_Premultiplied);

            int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
            int alpha = (GAUSSIAN_RADIUS < 1) ? 16 : (GAUSSIAN_RADIUS > 17) ? 1 : tab[GAUSSIAN_RADIUS - 1];

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

            QPainter painter(&CurrentImage);

            painter.setClipRegion(QRegion(blur_rect, QRegion::Ellipse));

            painter.drawImage(blur_rect, blur_image);

            if (IsLastBlurPointValid) {
                painter.setClipRegion(QRegion(last_blur_rect, QRegion::Ellipse));

                painter.drawImage(last_blur_rect, last_blur_image);
            }
        }

        IsChanged = true;

        RepaintImage(false, QRect(img_x, img_y, width, height));
        RepaintHelper(center_x, center_y);
    }
}

void RetouchEditor::updateHelperAt(int center_x, int center_y)
{
    if (CurrentMode != ModeScroll) {
        RepaintHelper(center_x, center_y);
    }
}

void RetouchEditor::undo()
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

void RetouchEditor::addFragment(int x, int y, bb::cascades::ImageView *fragment)
{
    FragmentsMap[QPair<int, int>(x, y)] = fragment;
}

void RetouchEditor::delFragment(int x, int y)
{
    FragmentsMap.remove(QPair<int, int>(x, y));
}

QList<QObject*> RetouchEditor::getFragments()
{
    QList<QObject*> result;

    for (int i = 0; i < FragmentsMap.keys().size(); i++) {
        result.append(FragmentsMap[FragmentsMap.keys().at(i)]);
    }

    return result;
}

void RetouchEditor::SaveUndoImage()
{
    UndoStack.push(CurrentImage);

    if (UndoStack.size() > UNDO_DEPTH) {
        for (int i = 0; i < UndoStack.size() - UNDO_DEPTH; i++) {
            UndoStack.remove(0);
        }
    }

    emit undoAvailabilityChanged(true);
}

void RetouchEditor::RepaintImage(bool full, QRect rect)
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

void RetouchEditor::RepaintHelper(int center_x, int center_y)
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
