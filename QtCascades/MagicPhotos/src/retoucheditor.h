#ifndef RETOUCHEDITOR_H
#define RETOUCHEDITOR_H

#include <QtCore/qmath.h>
#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QString>
#include <QtCore/QPair>
#include <QtCore/QList>
#include <QtCore/QStack>
#include <QtCore/QMap>
#include <QtGui/QImage>

#include <bb/cascades/ImageView>
#include <bb/cascades/CustomControl>

class RetouchEditor : public bb::cascades::CustomControl
{
    Q_OBJECT

    Q_PROPERTY(int                      mode               READ mode               WRITE setMode)
    Q_PROPERTY(int                      brushSize          READ brushSize          WRITE setBrushSize)
    Q_PROPERTY(int                      helperSize         READ helperSize         WRITE setHelperSize)
    Q_PROPERTY(bool                     samplingPointValid READ samplingPointValid WRITE setSamplingPointValid)
    Q_PROPERTY(bool                     lastBlurPointValid READ lastBlurPointValid WRITE setLastBlurPointValid)
    Q_PROPERTY(int                      samplingPointX     READ samplingPointX     WRITE setSamplingPointX)
    Q_PROPERTY(int                      samplingPointY     READ samplingPointY     WRITE setSamplingPointY)
    Q_PROPERTY(int                      lastBlurPointX     READ lastBlurPointX     WRITE setLastBlurPointX)
    Q_PROPERTY(int                      lastBlurPointY     READ lastBlurPointY     WRITE setLastBlurPointY)
    Q_PROPERTY(qreal                    brushOpacity       READ brushOpacity       WRITE setBrushOpacity)
    Q_PROPERTY(qreal                    scale              READ scale              WRITE setScale)
    Q_PROPERTY(qreal                    resolutionLimit    READ resolutionLimit    WRITE setResolutionLimit)
    Q_PROPERTY(bb::cascades::ImageView* helper             READ helper             WRITE setHelper)
    Q_PROPERTY(bool                     changed            READ changed)
    Q_PROPERTY(int                      imageWidth         READ imageWidth)
    Q_PROPERTY(int                      imageHeight        READ imageHeight)

    Q_ENUMS(Mode)

public:
    explicit RetouchEditor();
    virtual ~RetouchEditor();

    int  mode() const;
    void setMode(const int &mode);

    int  brushSize() const;
    void setBrushSize(const int &size);

    int  helperSize() const;
    void setHelperSize(const int &size);

    bool samplingPointValid() const;
    void setSamplingPointValid(const bool &valid);

    bool lastBlurPointValid() const;
    void setLastBlurPointValid(const bool &valid);

    int  samplingPointX() const;
    void setSamplingPointX(const int &x);

    int  samplingPointY() const;
    void setSamplingPointY(const int &y);

    int  lastBlurPointX() const;
    void setLastBlurPointX(const int &x);

    int  lastBlurPointY() const;
    void setLastBlurPointY(const int &y);

    qreal brushOpacity() const;
    void  setBrushOpacity(const qreal &opacity);

    qreal scale() const;
    void  setScale(const qreal &scale);

    qreal resolutionLimit() const;
    void  setResolutionLimit(const qreal &limit);

    bb::cascades::ImageView *helper() const;
    void                     setHelper(bb::cascades::ImageView *helper);

    bool changed() const;
    int  imageWidth() const;
    int  imageHeight() const;

    Q_INVOKABLE void openImage(const QString &image_file);
    Q_INVOKABLE void saveImage(const QString &image_file);

    Q_INVOKABLE void changeImageAt(bool save_undo, int center_x, int center_y);
    Q_INVOKABLE void updateHelperAt(int center_x, int center_y);

    Q_INVOKABLE void undo();

    Q_INVOKABLE void            addFragment(int x, int y, bb::cascades::ImageView *fragment);
    Q_INVOKABLE void            delFragment(int x, int y);
    Q_INVOKABLE QList<QObject*> getFragments();

    enum Mode {
        ModeScroll,
        ModeSamplingPoint,
        ModeClone,
        ModeBlur
    };

signals:
    void imageOpened();
    void imageOpenFailed();

    void imageSaved();
    void imageSaveFailed();

    void undoAvailabilityChanged(bool available);

    void prepareFragments(int fragmentSize, int imageWidth, int imageHeight);

private:
    void SaveUndoImage();

    void RepaintImage(bool full, QRect rect = QRect());
    void RepaintHelper(int center_x, int center_y);

    static const int UNDO_DEPTH      = 4,
                     GAUSSIAN_RADIUS = 4,
                     FRAGMENT_SIZE   = 64;

    bool                                            IsChanged, IsSamplingPointValid, IsLastBlurPointValid;
    int                                             CurrentMode, BrushSize, HelperSize;
    qreal                                           BrushOpacity, Scale, ResolutionLimit;
    QPoint                                          SamplingPoint, LastBlurPoint;
    QImage                                          LoadedImage, CurrentImage, BrushTemplateImage, BrushImage;
    QStack<QImage>                                  UndoStack;
    bb::cascades::ImageView                        *Helper;
    QMap<QPair<int, int>, bb::cascades::ImageView*> FragmentsMap;
};

#endif // RETOUCHEDITOR_H
