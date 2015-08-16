#include <QtGui/QImage>
#include <QtGui/QTransform>
#include <QtGui/QPainter>

#include "brushpreviewgenerator.h"

BrushPreviewGenerator::BrushPreviewGenerator(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    Size    = 0;
    MaxSize = 0;
    Opacity = 0.0;

    setRenderTarget(QQuickPaintedItem::FramebufferObject);

    setFlag(QQuickItem::ItemHasContents, true);
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

    update();
}

int BrushPreviewGenerator::maxSize() const
{
    return MaxSize;
}

void BrushPreviewGenerator::setMaxSize(const int &max_size)
{
    MaxSize = max_size;

    setImplicitWidth(MaxSize * 2);
    setImplicitHeight(MaxSize * 2);

    update();
}

qreal BrushPreviewGenerator::opacity() const
{
    return Opacity;
}

void BrushPreviewGenerator::setOpacity(const qreal &opacity)
{
    Opacity = opacity;

    update();
}

void BrushPreviewGenerator::paint(QPainter *painter)
{
    bool smooth_pixmap = painter->testRenderHint(QPainter::SmoothPixmapTransform);

    if (smooth()) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    } else {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
    }

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
            QImage image = result.scaled(QSize(contentsBoundingRect().width(),
                                               contentsBoundingRect().height()),
                                         Qt::KeepAspectRatio,
                                         smooth() ? Qt::SmoothTransformation : Qt::FastTransformation);

            painter->drawPixmap(QPoint((contentsBoundingRect().width()  - image.width())  / 2,
                                       (contentsBoundingRect().height() - image.height()) / 2), QPixmap::fromImage(image));
        }
    }

    painter->setRenderHint(QPainter::SmoothPixmapTransform, smooth_pixmap);
}
