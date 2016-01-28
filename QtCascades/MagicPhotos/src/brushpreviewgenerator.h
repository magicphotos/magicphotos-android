#ifndef BRUSHPREVIEWGENERATOR_H
#define BRUSHPREVIEWGENERATOR_H

#include <QtCore/qmath.h>
#include <QtCore/QObject>

#include <bb/cascades/ImageView>
#include <bb/cascades/CustomControl>

class BrushPreviewGenerator : public bb::cascades::CustomControl
{
    Q_OBJECT

    Q_PROPERTY(int                      size    READ size    WRITE setSize)
    Q_PROPERTY(int                      maxSize READ maxSize WRITE setMaxSize)
    Q_PROPERTY(qreal                    opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(bb::cascades::ImageView* preview READ preview WRITE setPreview)

public:
    explicit BrushPreviewGenerator();
    virtual ~BrushPreviewGenerator();

    int  size() const;
    void setSize(const int &size);

    int  maxSize() const;
    void setMaxSize(const int &max_size);

    qreal opacity() const;
    void  setOpacity(const qreal &opacity);

    bb::cascades::ImageView *preview() const;
    void                     setPreview(bb::cascades::ImageView *preview);

private:
    void Repaint();

    int                      Size, MaxSize;
    qreal                    Opacity;
    bb::cascades::ImageView *Preview;
};

#endif // BRUSHPREVIEWGENERATOR_H
