#include <QtGui/QImage>
#include <QtGui/QTransform>
#include <QtGui/QPainter>

#include <bb/ImageData>
#include <bb/cascades/Image>

#include "brushpreviewgenerator.h"

BrushPreviewGenerator::BrushPreviewGenerator() : bb::cascades::CustomControl()
{
    Size    = 0;
    MaxSize = 0;
    Opacity = 0.0;
    Preview = NULL;
}

BrushPreviewGenerator::~BrushPreviewGenerator()
{
}

int BrushPreviewGenerator::size() const
{
    return Size;
}

void BrushPreviewGenerator::setSize(const int &size)
{
    Size = size;

    Repaint();
}

int BrushPreviewGenerator::maxSize() const
{
    return MaxSize;
}

void BrushPreviewGenerator::setMaxSize(const int &max_size)
{
    MaxSize = max_size;

    Repaint();
}

qreal BrushPreviewGenerator::opacity() const
{
    return Opacity;
}

void BrushPreviewGenerator::setOpacity(const qreal &opacity)
{
    Opacity = opacity;

    Repaint();
}

bb::cascades::ImageView *BrushPreviewGenerator::preview() const
{
    return Preview;
}

void BrushPreviewGenerator::setPreview(bb::cascades::ImageView *preview)
{
    Preview = preview;
}

void BrushPreviewGenerator::Repaint()
{
    if (Size != 0 && MaxSize != 0) {
        QImage brush_template(Size * 2, Size * 2, QImage::Format_ARGB32);

        for (int x = 0; x < brush_template.width(); x++) {
            for (int y = 0; y < brush_template.height(); y++) {
                qreal r = qSqrt(qPow(x - Size, 2) + qPow(y - Size, 2));

                if (r <= Size) {
                    if (r <= Size * Opacity) {
                        brush_template.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, 0xFF));
                    } else {
                        brush_template.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, (int)(0xFF * (Size - r) / (Size * (1.0 - Opacity)))));
                    }
                } else {
                    brush_template.setPixel(x, y, qRgba(0xFF, 0xFF, 0xFF, 0x00));
                }
            }
        }

        QImage   brush_preview(brush_template.width(), brush_template.height(), QImage::Format_ARGB32);
        QPainter preview_painter(&brush_preview);

        brush_preview.fill(Qt::red);

        preview_painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        preview_painter.drawImage(QPoint(0, 0), brush_template);

        QImage   result(MaxSize * 2, MaxSize * 2, QImage::Format_ARGB32);
        QPainter result_painter(&result);

        result.fill(QColor::fromRgba(qRgba(0x00, 0x00, 0x00, 0x00)));

        result_painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        result_painter.drawImage(QPoint((result.width()  - brush_preview.width())  / 2,
                                        (result.height() - brush_preview.height()) / 2), brush_preview);

        if (!result.isNull()) {
            bb::ImageData image_data = bb::ImageData(bb::PixelFormat::RGBA_Premultiplied, result.width(), result.height());

            unsigned char *dst_line = image_data.pixels();

            for (int y = 0; y < image_data.height(); y++) {
                unsigned char *dst = dst_line;

                for (int x = 0; x < image_data.width(); x++) {
                    QRgb pixel = result.pixel(x, y);

                    *dst++ = qRed(pixel)   * (qAlpha(pixel) / 255.0);
                    *dst++ = qGreen(pixel) * (qAlpha(pixel) / 255.0);
                    *dst++ = qBlue(pixel)  * (qAlpha(pixel) / 255.0);
                    *dst++ = qAlpha(pixel);
                }

                dst_line += image_data.bytesPerLine();
            }

            if (Preview != NULL) {
                Preview->setImage(bb::cascades::Image(image_data));
            }
        }
    }
}
