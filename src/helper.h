#ifndef HELPER_H
#define HELPER_H

#include <QtGui/QImage>
#include <QtQuick/QQuickPaintedItem>

class Helper : public QQuickPaintedItem
{
    Q_OBJECT

public:
    explicit Helper(QQuickPaintedItem *parent = nullptr);
    ~Helper() override = default;

    void paint(QPainter *painter) override;

public slots:
    void helperImageReady(const QImage &helper_image);

private:
    QImage HelperImage;
};

#endif // HELPER_H
