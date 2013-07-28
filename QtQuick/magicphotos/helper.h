#ifndef HELPER_H
#define HELPER_H

#include <QImage>
#include <QDeclarativeItem>

class Helper : public QDeclarativeItem
{
    Q_OBJECT

public:
    explicit Helper(QDeclarativeItem *parent = 0);
    virtual ~Helper();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*);

public slots:
    void helperImageReady(const QImage &helper_image);

private:
    QImage HelperImage;
};

#endif // HELPER_H
