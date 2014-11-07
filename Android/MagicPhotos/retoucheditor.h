#ifndef RETOUCHEDITOR_H
#define RETOUCHEDITOR_H

#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QString>
#include <QtCore/QStack>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtQuick/QQuickPaintedItem>

class RetouchEditor : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(int    mode               READ mode               WRITE  setMode)
    Q_PROPERTY(int    brushSize          READ brushSize          WRITE  setBrushSize)
    Q_PROPERTY(int    helperSize         READ helperSize         WRITE  setHelperSize)
    Q_PROPERTY(bool   changed            READ changed)
    Q_PROPERTY(bool   samplingPointValid READ samplingPointValid NOTIFY samplingPointValidChanged)
    Q_PROPERTY(QPoint samplingPoint      READ samplingPoint      NOTIFY samplingPointChanged)

    Q_ENUMS(Mode)
    Q_ENUMS(MouseState)

public:
    explicit RetouchEditor(QQuickItem *parent = 0);
    virtual ~RetouchEditor();

    int  mode() const;
    void setMode(const int &mode);

    int  brushSize() const;
    void setBrushSize(const int &size);

    int  helperSize() const;
    void setHelperSize(const int &size);

    bool   changed() const;
    bool   samplingPointValid() const;
    QPoint samplingPoint() const;

    Q_INVOKABLE void openImage(const QString &image_file, const int &image_orientation);
    Q_INVOKABLE void saveImage(const QString &image_file);

    Q_INVOKABLE void undo();

    virtual void paint(QPainter *painter);

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

    void imageSaved(const QString &image_file);
    void imageSaveFailed();

    void undoAvailabilityChanged(bool available);

    void mouseEvent(int event_type, int x, int y);

    void helperImageReady(const QImage &helper_image);

    void samplingPointValidChanged();
    void samplingPointChanged();

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    void SaveUndoImage();
    void ChangeImageAt(bool save_undo, int center_x, int center_y);

    static const int UNDO_DEPTH      = 8,
                     GAUSSIAN_RADIUS = 4;

    constexpr static const qreal IMAGE_MPIX_LIMIT = 1.0;

    bool           IsChanged, IsSamplingPointValid, IsLastBlurPointValid;
    int            CurrentMode, BrushSize, HelperSize;
    QPoint         SamplingPoint, InitialSamplingPoint, LastBlurPoint, InitialTouchPoint;
    QImage         LoadedImage, CurrentImage;
    QStack<QImage> UndoStack;
};

#endif // RETOUCHEDITOR_H
