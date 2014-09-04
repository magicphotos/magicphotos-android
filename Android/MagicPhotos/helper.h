#ifndef HELPER_H
#define HELPER_H

#include <QtGui/QImage>
#include <QtQuick/QQuickPaintedItem>

class Helper : public QQuickPaintedItem
{
    Q_OBJECT

public:
    explicit Helper(QQuickPaintedItem *parent = 0);
    virtual ~Helper();

    virtual void paint(QPainter *painter);

public slots:
    void helperImageReady(const QImage &helper_image);

private:
    QImage HelperImage;
};

#endif // HELPER_H
