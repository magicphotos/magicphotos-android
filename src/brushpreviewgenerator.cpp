#include <QtCore/QtMath>
#include <QtCore/QSize>
#include <QtCore/QPoint>
#include <QtCore/QPointF>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QTransform>
#include <QtGui/QPainter>

#include "brushpreviewgenerator.h"

BrushPreviewGenerator::BrushPreviewGenerator(QQuickItem *parent) :
    QQuickPaintedItem(parent),
    BrushSize        (0),
    MaxBrushSize     (0),
    BrushHardness    (0.0)
{
    setFlag(QQuickItem::ItemHasContents, true);
}

int BrushPreviewGenerator::brushSize() const
{
    return BrushSize;
}

void BrushPreviewGenerator::setBrushSize(int size)
{
    BrushSize = size;

    update();
}

int BrushPreviewGenerator::maxBrushSize() const
{
    return MaxBrushSize;
}

void BrushPreviewGenerator::setMaxBrushSize(int max_size)
{
    MaxBrushSize = max_size;

    setImplicitWidth(MaxBrushSize * 2);
    setImplicitHeight(MaxBrushSize * 2);

    update();
}

qreal BrushPreviewGenerator::brushHardness() const
{
    return BrushHardness;
}

void BrushPreviewGenerator::setBrushHardness(qreal hardness)
{
    BrushHardness = hardness;

    update();
}

void BrushPreviewGenerator::paint(QPainter *painter)
{
    if (BrushSize != 0 && MaxBrushSize != 0) {
        QImage brush_template(BrushSize * 2, BrushSize * 2, QImage::Format_ARGB32);

        for (int y = 0; y < brush_template.height(); y++) {
            for (int x = 0; x < brush_template.width(); x++) {
                qreal r = qSqrt(qPow(x - BrushSize, 2) + qPow(y - BrushSize, 2));

                if (r <= BrushSize) {
                    if (r <= BrushSize * BrushHardness) {
                        brush_template.setPixel(x, y, qRgba(255, 255, 255, 255));
                    } else {
                        brush_template.setPixel(x, y, qRgba(255, 255, 255, qFloor(255 * (BrushSize - r) / (BrushSize * (1.0 - BrushHardness)))));
                    }
                } else {
                    brush_template.setPixel(x, y, qRgba(255, 255, 255, 0));
                }
            }
        }

        QImage   brush_preview(brush_template.width(), brush_template.height(), QImage::Format_ARGB32);
        QPainter preview_painter(&brush_preview);

        brush_preview.fill(Qt::red);

        preview_painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        preview_painter.drawImage(QPoint(0, 0), brush_template);

        QImage   result(MaxBrushSize * 2, MaxBrushSize * 2, QImage::Format_ARGB32);
        QPainter result_painter(&result);

        result.fill(QColor::fromRgba(qRgba(0, 0, 0, 0)));

        result_painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        result_painter.drawImage(QPoint((result.width()  - brush_preview.width())  / 2,
                                        (result.height() - brush_preview.height()) / 2), brush_preview);

        if (!result.isNull()) {
            QImage image = result.scaled(QSize(qFloor(width()), qFloor(height())), Qt::KeepAspectRatio,
                                         smooth() ? Qt::SmoothTransformation : Qt::FastTransformation);

            painter->drawImage(QPointF((width()  - image.width())  / 2,
                                       (height() - image.height()) / 2), image);
        }
    }
}
