#include <QtCore/QRectF>
#include <QtGui/QPainter>

#include "helper.h"

Helper::Helper(QQuickPaintedItem *parent) : QQuickPaintedItem(parent)
{
    setFlag(QQuickItem::ItemHasContents, true);
}

void Helper::setHelperImage(const QImage &image)
{
    HelperImage = image;

    update();
}

void Helper::paint(QPainter *painter)
{
    painter->drawImage(QRectF(0, 0, width(), height()), HelperImage, QRectF(0, 0, HelperImage.width(), HelperImage.height()));
}
