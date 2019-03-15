#include <QtGui/QPainter>

#include "helper.h"

Helper::Helper(QQuickPaintedItem *parent) : QQuickPaintedItem(parent)
{
    setFlag(QQuickItem::ItemHasContents, true);
}

void Helper::paint(QPainter *painter)
{
    painter->save();

    if (smooth()) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    } else {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
    }

    painter->drawPixmap(contentsBoundingRect(), QPixmap::fromImage(HelperImage), QRectF(0, 0, HelperImage.width(), HelperImage.height()));

    painter->restore();
}

void Helper::helperImageReady(const QImage &helper_image)
{
    HelperImage = helper_image;

    update();
}
