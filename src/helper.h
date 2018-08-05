#ifndef HELPER_H
#define HELPER_H

#include <QtGui/QImage>
#include <QtQuick/QQuickPaintedItem>

class Helper : public QQuickPaintedItem
{
    Q_OBJECT

public:
    explicit Helper(QQuickPaintedItem *parent = nullptr);
    virtual ~Helper();

    virtual void paint(QPainter *painter);

public slots:
    void helperImageReady(QImage helper_image);

private:
    QImage HelperImage;
};

#endif // HELPER_H
