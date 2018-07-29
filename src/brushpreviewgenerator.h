#ifndef BRUSHPREVIEWGENERATOR_H
#define BRUSHPREVIEWGENERATOR_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtQuick/QQuickPaintedItem>

class BrushPreviewGenerator : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(int   size    READ size    WRITE setSize)
    Q_PROPERTY(int   maxSize READ maxSize WRITE setMaxSize)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit BrushPreviewGenerator(QQuickItem *parent = 0);
    virtual ~BrushPreviewGenerator();

    int  size() const;
    void setSize(int size);

    int  maxSize() const;
    void setMaxSize(int max_size);

    qreal opacity() const;
    void  setOpacity(qreal opacity);

    virtual void paint(QPainter *painter);

private:
    int   Size, MaxSize;
    qreal Opacity;
};

#endif // BRUSHPREVIEWGENERATOR_H
