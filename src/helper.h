#ifndef HELPER_H
#define HELPER_H

#include <QtGui/QImage>
#include <QtQuick/QQuickPaintedItem>

class Helper : public QQuickPaintedItem
{
    Q_OBJECT

public:
    explicit Helper(QQuickPaintedItem *parent = nullptr);

    Helper(const Helper &) = delete;
    Helper(Helper &&) noexcept = delete;

    Helper &operator=(const Helper &) = delete;
    Helper &operator=(Helper &&) noexcept = delete;

    ~Helper() noexcept override = default;

    Q_INVOKABLE void setHelperImage(const QImage &image);

    void paint(QPainter *painter) override;

private:
    QImage HelperImage;
};

#endif // HELPER_H
