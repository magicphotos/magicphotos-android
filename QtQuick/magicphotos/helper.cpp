#include <QPainter>

#include "helper.h"

Helper::Helper(QDeclarativeItem *parent) : QDeclarativeItem(parent)
{
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);
    setFlag(QGraphicsItem::ItemHasNoContents,           false);
}

Helper::~Helper()
{
}

void Helper::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget*)
{
    qreal scale = 1.0;

    if (HelperImage.width() != 0) {
        scale = width() / HelperImage.width();
    }

    bool antialiasing = painter->testRenderHint(QPainter::Antialiasing);

    if (smooth()) {
        painter->setRenderHint(QPainter::Antialiasing, true);
    }

    QRectF source_rect(option->exposedRect.left()   / scale,
                       option->exposedRect.top()    / scale,
                       option->exposedRect.width()  / scale,
                       option->exposedRect.height() / scale);

    painter->drawImage(option->exposedRect, HelperImage.copy(source_rect.toRect()));

    painter->setRenderHint(QPainter::Antialiasing, antialiasing);
}

void Helper::helperImageReady(const QImage &helper_image)
{
    HelperImage = helper_image;

    update();
}
