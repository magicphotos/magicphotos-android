#ifndef PAINTITEM_H
#define PAINTITEM_H

#include <QString>
#include <QList>
#include <QHash>
#include <QStack>
#include <QGraphicsSceneMouseEvent>
#include <QImage>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QDeclarativeItem>

class PaintItem : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QString currentImageUrl READ currentImageUrl)

public:
    explicit PaintItem(QDeclarativeItem *parent = 0);
    virtual ~PaintItem();

    QString currentImageUrl() const;

    Q_INVOKABLE void setEffect(int effect);
    Q_INVOKABLE void setMode(int mode);
    Q_INVOKABLE void setScale(int scale);
    Q_INVOKABLE void setHue(int hue);

    Q_INVOKABLE void openImage(const QString &image_url);
    Q_INVOKABLE void openImageFromResource(const QString &image_res);

    Q_INVOKABLE void saveImage(const QString &image_url);

    Q_INVOKABLE void undo();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget*);

signals:
    void imageOpened();
    void imageOpenFailed();

    void imageSaved();
    void imageSaveFailed();

    void imageChanged();

    void undoStackEmpty();
    void undoStackNotEmpty();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
    union RGB16 {
        quint16 rgb;
        struct {
            unsigned r : 5;
            unsigned g : 6;
            unsigned b : 5;
        };
    };

    union HSV {
        quint32 hsv;
        struct {
            qint16 h;
            quint8 s;
            quint8 v;
        };
    };

    void PrepareImages();
    void MakeEffectedImages();
    void SaveUndoImage();
    void ChangeImageAt(int center_x, int center_y);

    QRgb AdjustHue(QRgb rgb);

    static const int EFFECT_GRAYSCALE = 0,
                     EFFECT_SKETCH    = 1,
                     EFFECT_BLUR      = 2,
                     EFFECT_HUE       = 3;

    static const int MODE_SCROLL   = 0,
                     MODE_ORIGINAL = 1,
                     MODE_EFFECTED = 2;

    static const int UNDO_DEPTH             = 4,
                     BRUSH_SIZE             = 16,
                     EVENT_PROCESS_INTERVAL = 500,
                     GAUSSIAN_RADIUS        = 16;

    static const qreal IMAGE_MPIX_LIMIT = 1.0;

    int                     CurrentEffect, CurrentMode, ScalePercent, Hue;
    QString                 CurrentImageURL;
    QImage                  LoadedImage, OriginalImage, EffectedImage, CurrentImage, ScaledOriginalImage, ScaledEffectedImage, ScaledCurrentImage;
    QList<QImage>           EffectedImages, CurrentImages;
    QHash<quint16, quint32> RGB16ToHSVMap;
    QStack<QImage>          UndoStack;
};

#endif // PAINTITEM_H
