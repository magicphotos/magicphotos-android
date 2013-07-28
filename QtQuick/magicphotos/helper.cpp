#include <QPainter>

#include "helper.h"

Helper::Helper(QDeclarativeItem *parent) : QDeclarativeItem(parent)
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);
}

Helper::~Helper()
{
}

void Helper::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    qreal scale = 1.0;

    if (HelperImage.width() != 0 && HelperImage.height() != 0) {
        scale = width() / HelperImage.width() < height() / HelperImage.height() ?
                width() / HelperImage.width() : height() / HelperImage.height();
    }

    bool antialiasing = painter->testRenderHint(QPainter::Antialiasing);

    if (smooth()) {
        painter->setRenderHint(QPainter::Antialiasing, true);
    }

    QRectF src_rect(0, 0,
                    HelperImage.width(),
                    HelperImage.height());
    QRectF dst_rect((width()  - HelperImage.width()  * scale) / 2,
                    (height() - HelperImage.height() * scale) / 2,
                    HelperImage.width()  * scale,
                    HelperImage.height() * scale);

    painter->drawImage(dst_rect, HelperImage, src_rect);

    painter->setRenderHint(QPainter::Antialiasing, antialiasing);
}

void Helper::helperImageReady(const QImage &helper_image)
{
    HelperImage = helper_image;

    update();
}
