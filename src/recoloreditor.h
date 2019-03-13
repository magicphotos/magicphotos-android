#ifndef RECOLOREDITOR_H
#define RECOLOREDITOR_H

#include <QtCore/QtGlobal>
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

    Q_PROPERTY(bool changed READ changed)

    Q_PROPERTY(int   mode         READ mode         WRITE setMode)
    Q_PROPERTY(int   brushSize    READ brushSize    WRITE setBrushSize)
    Q_PROPERTY(int   helperSize   READ helperSize   WRITE setHelperSize)
    Q_PROPERTY(int   hue          READ hue          WRITE setHue)
    Q_PROPERTY(qreal brushOpacity READ brushOpacity WRITE setBrushOpacity)

    Q_ENUMS(Mode)
    Q_ENUMS(MouseState)

public:
    explicit RecolorEditor(QQuickItem *parent = nullptr);
    ~RecolorEditor() override = default;

    bool changed() const;

    int mode() const;
    void setMode(int mode);

    int brushSize() const;
    void setBrushSize(int size);

    int helperSize() const;
    void setHelperSize(int size);

    int hue() const;
    void setHue(int hue);

    qreal brushOpacity() const;
    void setBrushOpacity(qreal opacity);

    Q_INVOKABLE void openImage(const QString &image_file, int image_orientation);
    Q_INVOKABLE void saveImage(const QString &image_file);

    Q_INVOKABLE void undo();

    void paint(QPainter *painter) override;

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

    void imageSaved(const QString &imageFile);
    void imageSaveFailed();

    void undoAvailabilityChanged(bool available);

    void mouseEvent(int eventType, int x, int y);

    void helperImageReady(const QImage &helperImage);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    union RGB16 {
        quint16 rgb;
        struct {
            unsigned int r : 5;
            unsigned int g : 6;
            unsigned int b : 5;
        } srgb;
    };

    union HSV {
        quint32 hsv;
        struct {
            qint16 h;
            quint8 s;
            quint8 v;
        } shsv;
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
