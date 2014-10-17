#include <QtCore/qmath.h>
#include <QtCore/QFileInfo>
#include <QtGui/QTransform>
#include <QtGui/QImageReader>
#include <QtGui/QPainter>

#include "retoucheditor.h"

RetouchEditor::RetouchEditor(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    IsChanged            = false;
    IsSamplingPointValid = false;
    IsLastBlurPointValid = false;
    CurrentMode          = ModeScroll;
    HelperSize           = 0;
    ScreenDPI            = 0;

    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);

    setRenderTarget(QQuickPaintedItem::FramebufferObject);

    setFlag(QQuickItem::ItemHasContents, true);
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

int RetouchEditor::helperSize() const
{
    return HelperSize;
}

void RetouchEditor::setHelperSize(const int &size)
{
    HelperSize = size;
}

int RetouchEditor::screenDPI() const
{
    return ScreenDPI;
}

void RetouchEditor::setScreenDPI(const int &dpi)
{
    ScreenDPI = dpi;
}

bool RetouchEditor::changed() const
{
    return IsChanged;
}

bool RetouchEditor::samplingPointValid() const
{
    return IsSamplingPointValid;
}

QPoint RetouchEditor::samplingPoint() const
{
    return SamplingPoint;
}

void RetouchEditor::openImage(const QString &image_file, const int &image_orientation)
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
                if (image_orientation == 90) {
                    QTransform transform;

                    transform.rotate(90);

                    LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.width(), LoadedImage.height());
                } else if (image_orientation == 180) {
                    QTransform transform;

                    transform.rotate(180);

                    LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
                } else if (image_orientation == 270) {
                    QTransform transform;

                    transform.rotate(270);

                    LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
                }

                LoadedImage = LoadedImage.convertToFormat(QImage::Format_RGB16);

                if (!LoadedImage.isNull()) {
                    CurrentImage = LoadedImage;

                    LoadedImage = QImage();

                    UndoStack.clear();

                    IsChanged            = false;
                    IsSamplingPointValid = false;

                    setImplicitWidth(CurrentImage.width());
                    setImplicitHeight(CurrentImage.height());

                    update();

                    emit samplingPointValidChanged();
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
    } else {
        emit imageOpenFailed();
    }
}

void RetouchEditor::saveImage(const QString &image_file)
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

void RetouchEditor::undo()
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

void RetouchEditor::paint(QPainter *painter)
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

void RetouchEditor::mousePressEvent(QMouseEvent *event)
{
    if (CurrentMode == ModeSamplingPoint) {
        int sampling_point_x = event->pos().x();
        int sampling_point_y = event->pos().y();

        if (sampling_point_x >= CurrentImage.width()) {
            sampling_point_x = CurrentImage.width() - 1;
        }
        if (sampling_point_y >= CurrentImage.height()) {
            sampling_point_y = CurrentImage.height() - 1;
        }
        if (sampling_point_x < 0) {
            sampling_point_x = 0;
        }
        if (sampling_point_y < 0) {
            sampling_point_y = 0;
        }

        IsSamplingPointValid = true;

        SamplingPoint.setX(sampling_point_x);
        SamplingPoint.setY(sampling_point_y);

        emit samplingPointValidChanged();
        emit samplingPointChanged();
    } else if (CurrentMode == ModeClone) {
        if (IsSamplingPointValid) {
            InitialSamplingPoint.setX(SamplingPoint.x());
            InitialSamplingPoint.setY(SamplingPoint.y());

            InitialTouchPoint.setX(event->pos().x());
            InitialTouchPoint.setY(event->pos().y());

            ChangeImageAt(true, event->pos().x(), event->pos().y());

            emit mouseEvent(MousePressed, event->pos().x(), event->pos().y());
        }
    } else if (CurrentMode == ModeBlur) {
        ChangeImageAt(true, event->pos().x(), event->pos().y());

        IsLastBlurPointValid = true;

        LastBlurPoint.setX(event->pos().x());
        LastBlurPoint.setY(event->pos().y());

        emit mouseEvent(MousePressed, event->pos().x(), event->pos().y());
    }
}

void RetouchEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (CurrentMode == ModeSamplingPoint) {
        int sampling_point_x = event->pos().x();
        int sampling_point_y = event->pos().y();

        if (sampling_point_x >= CurrentImage.width()) {
            sampling_point_x = CurrentImage.width() - 1;
        }
        if (sampling_point_y >= CurrentImage.height()) {
            sampling_point_y = CurrentImage.height() - 1;
        }
        if (sampling_point_x < 0) {
            sampling_point_x = 0;
        }
        if (sampling_point_y < 0) {
            sampling_point_y = 0;
        }

        IsSamplingPointValid = true;

        SamplingPoint.setX(sampling_point_x);
        SamplingPoint.setY(sampling_point_y);

        emit samplingPointValidChanged();
        emit samplingPointChanged();
    } else if (CurrentMode == ModeClone) {
        if (IsSamplingPointValid) {
            int sampling_point_x = InitialSamplingPoint.x() + (event->pos().x() - InitialTouchPoint.x());
            int sampling_point_y = InitialSamplingPoint.y() + (event->pos().y() - InitialTouchPoint.y());

            if (sampling_point_x >= CurrentImage.width()) {
                sampling_point_x = CurrentImage.width() - 1;
            }
            if (sampling_point_y >= CurrentImage.height()) {
                sampling_point_y = CurrentImage.height() - 1;
            }
            if (sampling_point_x < 0) {
                sampling_point_x = 0;
            }
            if (sampling_point_y < 0) {
                sampling_point_y = 0;
            }

            SamplingPoint.setX(sampling_point_x);
            SamplingPoint.setY(sampling_point_y);

            emit samplingPointChanged();

            ChangeImageAt(false, event->pos().x(), event->pos().y());

            emit mouseEvent(MouseMoved, event->pos().x(), event->pos().y());
        }
    } else if (CurrentMode == ModeBlur) {
        ChangeImageAt(false, event->pos().x(), event->pos().y());

        LastBlurPoint.setX(event->pos().x());
        LastBlurPoint.setY(event->pos().y());

        emit mouseEvent(MouseMoved, event->pos().x(), event->pos().y());
    }
}

void RetouchEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (CurrentMode == ModeClone) {
        emit mouseEvent(MouseReleased, event->pos().x(), event->pos().y());
    } else if (CurrentMode == ModeBlur) {
        IsLastBlurPointValid = false;

        emit mouseEvent(MouseReleased, event->pos().x(), event->pos().y());
    }
}

int RetouchEditor::MapSizeToDevice(int size)
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

void RetouchEditor::ChangeImageAt(bool save_undo, int center_x, int center_y)
{
    if (CurrentMode == ModeClone || CurrentMode == ModeBlur) {
        if (save_undo) {
            SaveUndoImage();
        }

        int radius = MapSizeToDevice(BRUSH_SIZE) / scale();

        if (CurrentMode == ModeClone) {
            for (int from_x = SamplingPoint.x() - radius, to_x = center_x - radius; from_x <= SamplingPoint.x() + radius && to_x <= center_x + radius; from_x++, to_x++) {
                for (int from_y = SamplingPoint.y() - radius, to_y = center_y - radius; from_y <= SamplingPoint.y() + radius && to_y <= center_y + radius; from_y++, to_y++) {
                    if (from_x >= 0 && from_x < CurrentImage.width() && from_y >= 0 && from_y < CurrentImage.height() && qSqrt(qPow(from_x - SamplingPoint.x(), 2) + qPow(from_y - SamplingPoint.y(), 2)) <= radius &&
                        to_x   >= 0 && to_x   < CurrentImage.width() && to_y   >= 0 && to_y   < CurrentImage.height() && qSqrt(qPow(to_x   - center_x,      2) + qPow(to_y   - center_y,      2)) <= radius) {
                        CurrentImage.setPixel(to_x, to_y, CurrentImage.pixel(from_x, from_y));
                    }
                }
            }
        } else if (CurrentMode == ModeBlur) {
            QRect  last_blur_rect(LastBlurPoint.x() - radius, LastBlurPoint.y() - radius, radius * 2, radius * 2);
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

            QRect blur_rect(center_x - radius, center_y - radius, radius * 2, radius * 2);

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

        update();

        QImage helper_image = CurrentImage.copy(center_x - (HelperSize / scale()) / 2,
                                                center_y - (HelperSize / scale()) / 2,
                                                HelperSize / scale(),
                                                HelperSize / scale()).scaledToWidth(HelperSize);

        emit helperImageReady(helper_image);
    }
}
