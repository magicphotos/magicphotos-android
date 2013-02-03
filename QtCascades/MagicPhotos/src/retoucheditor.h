#ifndef RETOUCHEDITOR_H
#define RETOUCHEDITOR_H

#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QStack>
#include <QtCore/QHash>
#include <QtGui/QImage>

#include <bb/ImageData>
#include <bb/cascades/Image>
#include <bb/cascades/CustomControl>

class RetouchEditor : public bb::cascades::CustomControl
{
    Q_OBJECT

    Q_PROPERTY(int  mode               READ mode               WRITE setMode)
    Q_PROPERTY(bool samplingPointValid READ samplingPointValid WRITE setSamplingPointValid)
    Q_PROPERTY(int  samplingPointX     READ samplingPointX     WRITE setSamplingPointX)
    Q_PROPERTY(int  samplingPointY     READ samplingPointY     WRITE setSamplingPointY)
    Q_PROPERTY(bool changed            READ changed)
    Q_PROPERTY(int  imageWidth         READ imageWidth)
    Q_PROPERTY(int  imageHeight        READ imageHeight)

    Q_ENUMS(Mode)

public:
    explicit RetouchEditor();
    virtual ~RetouchEditor();

    int  mode() const;
    void setMode(const int &mode);

    bool samplingPointValid() const;
    void setSamplingPointValid(const bool &valid);

    int  samplingPointX() const;
    void setSamplingPointX(const int &x);

    int  samplingPointY() const;
    void setSamplingPointY(const int &y);

    bool changed() const;
    int  imageWidth() const;
    int  imageHeight() const;

    Q_INVOKABLE void openImage(const QString &image_file);
    Q_INVOKABLE void saveImage(const QString &image_file);

    Q_INVOKABLE void changeImageAt(bool save_undo, int center_x, int center_y, double zoom_level);
    Q_INVOKABLE void updateHelperAt(int center_x, int center_y, double zoom_level);

    Q_INVOKABLE void undo();

    enum Mode {
        ModeScroll,
        ModeSamplingPoint,
        ModeClone
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
    void SaveUndoImage();

    void RepaintImage(bool full, QRect rect = QRect());
    void RepaintHelper(int center_x, int center_y, double zoom_level);

    static const int UNDO_DEPTH      = 4,
                     BRUSH_SIZE      = 32,
                     GAUSSIAN_RADIUS = 4,
                     HELPER_SIZE     = 192;

    static const qreal IMAGE_MPIX_LIMIT = 1.0;

    bool           IsChanged, IsSamplingPointValid;
    int            CurrentMode;
    QPoint         SamplingPoint;
    QImage         LoadedImage, CurrentImage;
    QStack<QImage> UndoStack;
    bb::ImageData  CurrentImageData;
};

#endif // RETOUCHEDITOR_H
