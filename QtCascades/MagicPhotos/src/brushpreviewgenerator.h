#ifndef BRUSHPREVIEWGENERATOR_H
#define BRUSHPREVIEWGENERATOR_H

#include <QtCore/qmath.h>
#include <QtCore/QObject>

#include <bb/ImageData>
#include <bb/cascades/Image>
#include <bb/cascades/CustomControl>

class BrushPreviewGenerator : public bb::cascades::CustomControl
{
    Q_OBJECT

    Q_PROPERTY(int   size    READ size    WRITE setSize)
    Q_PROPERTY(int   maxSize READ maxSize WRITE setMaxSize)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit BrushPreviewGenerator();
    virtual ~BrushPreviewGenerator();

    int  size() const;
    void setSize(const int &size);

    int  maxSize() const;
    void setMaxSize(const int &max_size);

    qreal opacity() const;
    void  setOpacity(const qreal &opacity);

signals:
    void needRepaint(const bb::cascades::Image &image);

private:
    void Repaint();

    int   Size, MaxSize;
    qreal Opacity;
};

#endif // BRUSHPREVIEWGENERATOR_H
