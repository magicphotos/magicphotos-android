#ifndef RETOUCHEDITOR_H
#define RETOUCHEDITOR_H

#include <QObject>
#include <QPoint>
#include <QString>
#include <QStack>
#include <QImage>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QDeclarativeItem>

class RetouchEditor : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(int    mode               READ mode               WRITE  setMode)
    Q_PROPERTY(int    helperSize         READ helperSize         WRITE  setHelperSize)
    Q_PROPERTY(bool   changed            READ changed)
    Q_PROPERTY(bool   samplingPointValid READ samplingPointValid NOTIFY samplingPointValidChanged)
    Q_PROPERTY(QPoint samplingPoint      READ samplingPoint      NOTIFY samplingPointChanged)

    Q_ENUMS(Mode)
    Q_ENUMS(MouseState)

public:
    explicit RetouchEditor(QDeclarativeItem *parent = 0);
    virtual ~RetouchEditor();

    int  mode() const;
    void setMode(const int &mode);

    int  helperSize() const;
    void setHelperSize(const int &size);

    bool   changed() const;
    bool   samplingPointValid() const;
    QPoint samplingPoint() const;

    Q_INVOKABLE void openImage(const QString &image_url);
    Q_INVOKABLE void saveImage(const QString &image_url);

    Q_INVOKABLE void undo();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget*);

    enum Mode {
        ModeScroll,
        ModeSamplingPoint,
        ModeClone,
        ModeBlur
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

    void samplingPointValidChanged();
    void samplingPointChanged();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    void SaveUndoImage();
    void ChangeImageAt(bool save_undo, int center_x, int center_y);

    static const int UNDO_DEPTH      = 4,
                     BRUSH_SIZE      = 16,
                     GAUSSIAN_RADIUS = 4;

    static const qreal IMAGE_MPIX_LIMIT = 1.0;

    bool           IsChanged, IsSamplingPointValid, IsLastBlurPointValid;
    int            CurrentMode, HelperSize;
    QPoint         SamplingPoint, InitialSamplingPoint, LastBlurPoint, InitialTouchPoint;
    QImage         LoadedImage, CurrentImage;
    QStack<QImage> UndoStack;
};

#endif // RETOUCHEDITOR_H
