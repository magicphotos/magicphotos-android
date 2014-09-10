#ifndef SKETCHEDITOR_H
#define SKETCHEDITOR_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStack>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtQuick/QQuickPaintedItem>

class SketchEditor : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(int  mode               READ mode               WRITE setMode)
    Q_PROPERTY(int  helperSize         READ helperSize         WRITE setHelperSize)
    Q_PROPERTY(int  screenPixelDensity READ screenPixelDensity WRITE setScreenPixelDensity)
    Q_PROPERTY(int  radius             READ radius             WRITE setRadius)
    Q_PROPERTY(bool changed            READ changed)

    Q_ENUMS(Mode)
    Q_ENUMS(MouseState)

public:
    explicit SketchEditor(QQuickItem *parent = 0);
    virtual ~SketchEditor();

    int  mode() const;
    void setMode(const int &mode);

    int  helperSize() const;
    void setHelperSize(const int &size);

    int  screenPixelDensity() const;
    void setScreenPixelDensity(const int &density);

    int  radius() const;
    void setRadius(const int &radius);

    bool changed() const;

    Q_INVOKABLE void openImage(const QString &image_file);
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

public slots:
    void effectedImageReady(const QImage &effected_image);

signals:
    void imageOpened();
    void imageOpenFailed();

    void imageSaved();
    void imageSaveFailed();

    void undoAvailabilityChanged(bool available);

    void mouseEvent(int event_type, int x, int y);

    void helperImageReady(const QImage &helper_image);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    int  MapSizeToDevice(int size);
    void SaveUndoImage();
    void ChangeImageAt(bool save_undo, int center_x, int center_y);

    static const int UNDO_DEPTH = 4,
                     BRUSH_SIZE = 16;

    constexpr static const qreal IMAGE_MPIX_LIMIT = 1.0;

    bool           IsChanged;
    int            CurrentMode, HelperSize, ScreenPixelDensity, GaussianRadius;
    QImage         LoadedImage, OriginalImage, EffectedImage, CurrentImage;
    QStack<QImage> UndoStack;
};

class SketchPreviewGenerator : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(int radius READ radius WRITE setRadius)

public:
    explicit SketchPreviewGenerator(QQuickItem *parent = 0);
    virtual ~SketchPreviewGenerator();

    int  radius() const;
    void setRadius(const int &radius);

    Q_INVOKABLE void openImage(const QString &image_file);

    virtual void paint(QPainter *painter);

public slots:
    void sketchImageReady(const QImage &sketch_image);

signals:
    void imageOpened();
    void imageOpenFailed();

    void generationStarted();
    void generationFinished();

private:
    void StartSketchGenerator();

    constexpr static const qreal IMAGE_MPIX_LIMIT = 0.2;

    bool   SketchGeneratorRunning, RestartSketchGenerator;
    int    GaussianRadius;
    QImage LoadedImage, SketchImage;
};

class SketchImageGenerator : public QObject
{
    Q_OBJECT

public:
    explicit SketchImageGenerator(QObject *parent = 0);
    virtual ~SketchImageGenerator();

    void setGaussianRadius(const int &radius);
    void setInput(const QImage &input_image);

public slots:
    void start();

signals:
    void imageReady(const QImage &output_image);
    void finished();

private:
    int    GaussianRadius;
    QImage InputImage;
};

#endif // SKETCHEDITOR_H
