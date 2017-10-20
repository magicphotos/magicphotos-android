#ifndef RECOLOREDITOR_H
#define RECOLOREDITOR_H

#include <QtCore/qmath.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStack>
#include <QtCore/QHash>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtQuick/QQuickPaintedItem>

class RecolorEditor : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(int   mode         READ mode         WRITE setMode)
    Q_PROPERTY(int   brushSize    READ brushSize    WRITE setBrushSize)
    Q_PROPERTY(int   helperSize   READ helperSize   WRITE setHelperSize)
    Q_PROPERTY(int   hue          READ hue          WRITE setHue)
    Q_PROPERTY(qreal brushOpacity READ brushOpacity WRITE setBrushOpacity)
    Q_PROPERTY(bool  changed      READ changed)

    Q_ENUMS(Mode)
    Q_ENUMS(MouseState)

public:
    explicit RecolorEditor(QQuickItem *parent = 0);
    virtual ~RecolorEditor();

    int  mode() const;
    void setMode(const int &mode);

    int  brushSize() const;
    void setBrushSize(const int &size);

    int  helperSize() const;
    void setHelperSize(const int &size);

    int  hue() const;
    void setHue(const int &hue);

    qreal brushOpacity() const;
    void  setBrushOpacity(const qreal &opacity);

    bool changed() const;

    Q_INVOKABLE void openImage(const QString &image_file, const int &image_orientation);
    Q_INVOKABLE void saveImage(const QString &image_file);

    Q_INVOKABLE void undo();

    virtual void paint(QPainter *painter);

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

private slots:
    void scaleWasChanged();

signals:
    void imageOpened();
    void imageOpenFailed();

    void imageSaved(const QString &image_file);
    void imageSaveFailed();

    void undoAvailabilityChanged(bool available);

    void mouseEvent(int event_type, int x, int y);

    void helperImageReady(const QImage &helper_image);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

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

    static const int UNDO_DEPTH = 8;

    constexpr static const qreal IMAGE_MPIX_LIMIT = 1.0;

    bool                    IsChanged;
    int                     CurrentMode, BrushSize, HelperSize, CurrentHue;
    qreal                   BrushOpacity;
    QImage                  LoadedImage, OriginalImage, CurrentImage, BrushTemplateImage, BrushImage;
    QStack<QImage>          UndoStack;
    QHash<quint16, quint32> RGB16ToHSVMap;
};

#endif // RECOLOREDITOR_H
