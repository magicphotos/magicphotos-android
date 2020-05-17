#include <array>

#include <QtCore/QtMath>
#include <QtCore/QRect>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QPainter>

#include "retoucheditor.h"

RetouchEditor::RetouchEditor(QQuickItem *parent) :
    Editor            (parent),
    SamplingPointValid(false),
    LastBlurPointValid(false)
{
}

bool RetouchEditor::samplingPointValid() const
{
    return SamplingPointValid;
}

QPoint RetouchEditor::samplingPoint() const
{
    return SamplingPoint;
}

void RetouchEditor::mousePressEvent(QMouseEvent *event)
{
    if (Mode == ModeSamplingPoint) {
        int sampling_point_x = event->pos().x();
        int sampling_point_y = event->pos().y();

        if (!SamplingPointValid) {
            SamplingPointValid = true;

            emit samplingPointValidChanged(SamplingPointValid);
        }

        if (SamplingPoint.x() != sampling_point_x ||
            SamplingPoint.y() != sampling_point_y) {
            SamplingPoint.setX(sampling_point_x);
            SamplingPoint.setY(sampling_point_y);

            emit samplingPointChanged(SamplingPoint);
        }
    } else if (Mode == ModeClone) {
        if (SamplingPointValid) {
            InitialSamplingPoint.setX(SamplingPoint.x());
            InitialSamplingPoint.setY(SamplingPoint.y());

            InitialTouchPoint.setX(event->pos().x());
            InitialTouchPoint.setY(event->pos().y());

            ChangeImageAt(true, event->pos().x(), event->pos().y());

            emit mouseEvent(MousePressed, event->pos().x(), event->pos().y());
        }
    } else if (Mode == ModeBlur) {
        ChangeImageAt(true, event->pos().x(), event->pos().y());

        LastBlurPointValid = true;

        LastBlurPoint.setX(event->pos().x());
        LastBlurPoint.setY(event->pos().y());

        emit mouseEvent(MousePressed, event->pos().x(), event->pos().y());
    }
}

void RetouchEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (Mode == ModeSamplingPoint) {
        int sampling_point_x = event->pos().x();
        int sampling_point_y = event->pos().y();

        if (!SamplingPointValid) {
            SamplingPointValid = true;

            emit samplingPointValidChanged(SamplingPointValid);
        }

        if (SamplingPoint.x() != sampling_point_x ||
            SamplingPoint.y() != sampling_point_y) {
            SamplingPoint.setX(sampling_point_x);
            SamplingPoint.setY(sampling_point_y);

            emit samplingPointChanged(SamplingPoint);
        }
    } else if (Mode == ModeClone) {
        if (SamplingPointValid) {
            int sampling_point_x = InitialSamplingPoint.x() + (event->pos().x() - InitialTouchPoint.x());
            int sampling_point_y = InitialSamplingPoint.y() + (event->pos().y() - InitialTouchPoint.y());

            if (SamplingPoint.x() != sampling_point_x ||
                SamplingPoint.y() != sampling_point_y) {
                SamplingPoint.setX(sampling_point_x);
                SamplingPoint.setY(sampling_point_y);

                emit samplingPointChanged(SamplingPoint);
            }

            ChangeImageAt(false, event->pos().x(), event->pos().y());

            emit mouseEvent(MouseMoved, event->pos().x(), event->pos().y());
        }
    } else if (Mode == ModeBlur) {
        ChangeImageAt(false, event->pos().x(), event->pos().y());

        LastBlurPoint.setX(event->pos().x());
        LastBlurPoint.setY(event->pos().y());

        emit mouseEvent(MouseMoved, event->pos().x(), event->pos().y());
    }
}

void RetouchEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (Mode == ModeClone) {
        emit mouseEvent(MouseReleased, event->pos().x(), event->pos().y());
    } else if (Mode == ModeBlur) {
        LastBlurPointValid = false;

        emit mouseEvent(MouseReleased, event->pos().x(), event->pos().y());
    }
}

void RetouchEditor::ProcessOpenedImage()
{
    CurrentImage = LoadedImage;

    LoadedImage = QImage();

    Changed = false;

    if (SamplingPointValid) {
        SamplingPointValid = false;

        emit samplingPointValidChanged(SamplingPointValid);
    }

    setImplicitWidth(CurrentImage.width());
    setImplicitHeight(CurrentImage.height());

    update();

    emit scaleChanged();
    emit imageOpened();
}

void RetouchEditor::ChangeImageAt(bool save_undo, int center_x, int center_y)
{
    if (Mode == ModeClone || Mode == ModeBlur) {
        if (save_undo) {
            SaveUndoImage();
        }

        int width  = BrushImage.width();
        int height = BrushImage.height();

        int img_x = center_x - width  / 2;
        int img_y = center_y - height / 2;

        if (Mode == ModeClone) {
            int src_x = SamplingPoint.x() - width  / 2;
            int src_y = SamplingPoint.y() - height / 2;

            QImage brush_image(width, height, QImage::Format_ARGB32);

            brush_image.fill(qRgba(0, 0, 0, 0));

            QPainter brush_painter(&brush_image);

            brush_painter.setCompositionMode(QPainter::CompositionMode_Source);
            brush_painter.drawImage(QPoint(0, 0), CurrentImage, QRect(src_x, src_y, width, height));

            brush_painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            brush_painter.drawImage(QPoint(0, 0), BrushImage);

            QPainter image_painter(&CurrentImage);

            image_painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            image_painter.drawImage(QPoint(img_x, img_y), brush_image);
        } else if (Mode == ModeBlur) {
            QRect  last_blur_rect(LastBlurPoint.x() - width / 2, LastBlurPoint.y() - height / 2, width, height);
            QImage last_blur_image;

            if (LastBlurPointValid) {
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

            constexpr std::array<int, 17> tab = {14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2};

            int alpha = BLUR_RADIUS < 1 ? 16 : (BLUR_RADIUS > 17 ? 1 : tab[BLUR_RADIUS - 1]);

            int r1 = blur_image.rect().top();
            int r2 = blur_image.rect().bottom();
            int c1 = blur_image.rect().left();
            int c2 = blur_image.rect().right();

            int bpl = blur_image.bytesPerLine();

            std::array<int, 4> rgba = {};
            unsigned char     *p;

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

            QPainter painter(&CurrentImage);

            painter.setClipRegion(QRegion(blur_rect, QRegion::Ellipse));

            painter.drawImage(blur_rect, blur_image);

            if (LastBlurPointValid) {
                painter.setClipRegion(QRegion(last_blur_rect, QRegion::Ellipse));

                painter.drawImage(last_blur_rect, last_blur_image);
            }
        }

        Changed = true;

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
