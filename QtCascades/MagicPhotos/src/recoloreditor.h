#ifndef RECOLOREDITOR_H
#define RECOLOREDITOR_H

#include <QtCore/qmath.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QPair>
#include <QtCore/QList>
#include <QtCore/QStack>
#include <QtCore/QMap>
#include <QtCore/QHash>
#include <QtGui/QImage>

#include <bb/cascades/ImageView>
#include <bb/cascades/CustomControl>

class RecolorEditor : public bb::cascades::CustomControl
{
    Q_OBJECT

    Q_PROPERTY(int                      mode            READ mode            WRITE setMode)
    Q_PROPERTY(int                      brushSize       READ brushSize       WRITE setBrushSize)
    Q_PROPERTY(int                      helperSize      READ helperSize      WRITE setHelperSize)
    Q_PROPERTY(int                      hue             READ hue             WRITE setHue)
    Q_PROPERTY(qreal                    brushOpacity    READ brushOpacity    WRITE setBrushOpacity)
    Q_PROPERTY(qreal                    scale           READ scale           WRITE setScale)
    Q_PROPERTY(qreal                    resolutionLimit READ resolutionLimit WRITE setResolutionLimit)
    Q_PROPERTY(bb::cascades::ImageView* helper          READ helper          WRITE setHelper)
    Q_PROPERTY(bool                     changed         READ changed)

    Q_ENUMS(Mode)

public:
    explicit RecolorEditor();
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

    qreal scale() const;
    void  setScale(const qreal &scale);

    qreal resolutionLimit() const;
    void  setResolutionLimit(const qreal &limit);

    bb::cascades::ImageView *helper() const;
    void                     setHelper(bb::cascades::ImageView *helper);

    bool changed() const;

    Q_INVOKABLE void openImage(const QString &image_file);
    Q_INVOKABLE void saveImage(const QString &image_file);

    Q_INVOKABLE void changeImageAt(bool save_undo, int center_x, int center_y);

    Q_INVOKABLE void undo();

    Q_INVOKABLE void            addFragment(int x, int y, bb::cascades::ImageView *fragment);
    Q_INVOKABLE void            delFragment(int x, int y);
    Q_INVOKABLE QList<QObject*> getFragments();

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

    void prepareFragments(int fragmentSize, int imageWidth, int imageHeight);

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
    void RepaintHelper(int center_x, int center_y);

    static const int UNDO_DEPTH    = 4,
                     FRAGMENT_SIZE = 64;

    bool                                            IsChanged;
    int                                             CurrentMode, BrushSize, HelperSize, CurrentHue;
    qreal                                           BrushOpacity, Scale, ResolutionLimit;
    QImage                                          LoadedImage, OriginalImage, CurrentImage, BrushTemplateImage, BrushImage;
    QStack<QImage>                                  UndoStack;
    QHash<quint16, quint32>                         RGB16ToHSVMap;
    bb::cascades::ImageView                        *Helper;
    QMap<QPair<int, int>, bb::cascades::ImageView*> FragmentsMap;
};

#endif // RECOLOREDITOR_H
