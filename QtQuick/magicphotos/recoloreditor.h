#ifndef RECOLOREDITOR_H
#define RECOLOREDITOR_H

#include <QObject>
#include <QString>
#include <QStack>
#include <QHash>
#include <QImage>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QDeclarativeItem>

class RecolorEditor : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(int  mode       READ mode       WRITE setMode)
    Q_PROPERTY(int  helperSize READ helperSize WRITE setHelperSize)
    Q_PROPERTY(int  hue        READ hue        WRITE setHue)
    Q_PROPERTY(bool changed    READ changed)

    Q_ENUMS(Mode)
    Q_ENUMS(MouseState)

public:
    explicit RecolorEditor(QDeclarativeItem *parent = 0);
    virtual ~RecolorEditor();

    int  mode() const;
    void setMode(const int &mode);

    int  helperSize() const;
    void setHelperSize(const int &size);

    int  hue() const;
    void setHue(const int &hue);

    bool changed() const;

    Q_INVOKABLE void openImage(const QString &image_url);
    Q_INVOKABLE void saveImage(const QString &image_url);

    Q_INVOKABLE void undo();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget*);

    enum Mode {
        ModeScroll,
        ModeOriginal,
        ModeEffected
    };

    enum MouseState {
        MousePressed,
        MouseMoved,
        MouseReleased
    };

signals:
    void imageOpened();
    void imageOpenFailed();

    void imageSaved();
    void imageSaveFailed();

    void undoAvailabilityChanged(bool available);

    void mouseEvent(int event_type, int x, int y);

    void helperImageReady(const QImage &helper_image);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

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

    QRgb AdjustHue(QRgb rgb);

    void SaveUndoImage();
    void ChangeImageAt(bool save_undo, int center_x, int center_y);

    static const int UNDO_DEPTH  = 4,
                     BRUSH_SIZE  = 16;

    static const qreal IMAGE_MPIX_LIMIT = 1.0;

    bool                    IsChanged;
    int                     CurrentMode, HelperSize, CurrentHue;
    QImage                  LoadedImage, OriginalImage, CurrentImage;
    QStack<QImage>          UndoStack;
    QHash<quint16, quint32> RGB16ToHSVMap;
};

#endif // RECOLOREDITOR_H
