#ifndef RECOLOREDITOR_H
#define RECOLOREDITOR_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStack>
#include <QtCore/QHash>
#include <QtGui/QImage>

#include <bb/ImageData>
#include <bb/cascades/Image>
#include <bb/cascades/CustomControl>

class RecolorEditor : public bb::cascades::CustomControl
{
    Q_OBJECT

    Q_PROPERTY(int  mode    READ mode   WRITE setMode)
    Q_PROPERTY(int  hue     READ hue    WRITE setHue)
    Q_PROPERTY(bool changed READ changed)

    Q_ENUMS(Mode)

public:
    explicit RecolorEditor();
    virtual ~RecolorEditor();

    int  mode() const;
    void setMode(const int &mode);

    int  hue() const;
    void setHue(const int &hue);

    bool changed() const;

    Q_INVOKABLE void openImage(const QString &image_file);
    Q_INVOKABLE void saveImage(const QString &image_file);

    Q_INVOKABLE void changeImageAt(bool save_undo, int center_x, int center_y, double zoom_level);

    Q_INVOKABLE void undo();

    enum Mode {
        ModeScroll,
        ModeOriginal,
        ModeEffected,
        ModeHueSelection
    };

signals:
    void imageOpened();
    void imageOpenFailed();

    void imageSaved();
    void imageSaveFailed();

    void undoAvailabilityChanged(bool available);

    void needImageRepaint(const bb::cascades::Image &image);
    void needHelperRepaint(const bb::cascades::Image &image);

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

    void SaveUndoImage();

    QRgb AdjustHue(QRgb rgb);

    void RepaintImage(bool full, QRect rect = QRect());
    void RepaintHelper(int center_x, int center_y, double zoom_level);

    static const int UNDO_DEPTH  = 4,
                     BRUSH_SIZE  = 32,
                     HELPER_SIZE = 192;

    static const qreal IMAGE_MPIX_LIMIT = 1.0;

    bool                    IsChanged;
    int                     CurrentMode, CurrentHue;
    QImage                  LoadedImage, OriginalImage, CurrentImage;
    QStack<QImage>          UndoStack;
    QHash<quint16, quint32> RGB16ToHSVMap;
    bb::ImageData           CurrentImageData;
};

#endif // RECOLOREDITOR_H
