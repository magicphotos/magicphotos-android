#include <QtGui/QPainter>

#include "helper.h"

Helper::Helper(QQuickPaintedItem *parent) : QQuickPaintedItem(parent)
{
    setFlag(QQuickItem::ItemHasContents, true);
}

Helper::~Helper()
{
}

void Helper::paint(QPainter *painter)
{
    bool smooth_pixmap = painter->testRenderHint(QPainter::SmoothPixmapTransform);

    if (smooth()) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    } else {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
    }

    painter->drawPixmap(contentsBoundingRect(), QPixmap::fromImage(HelperImage), QRectF(0, 0, HelperImage.width(), HelperImage.height()));

    painter->setRenderHint(QPainter::SmoothPixmapTransform, smooth_pixmap);
}

void Helper::helperImageReady(QImage helper_image)
{
    HelperImage = helper_image;

    update();
}
