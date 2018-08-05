#ifndef SKETCHEDITOR_H
#define SKETCHEDITOR_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStack>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtQuick/QQuickPaintedItem>

class SketchEditor : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(int   mode         READ mode         WRITE setMode)
    Q_PROPERTY(int   brushSize    READ brushSize    WRITE setBrushSize)
    Q_PROPERTY(int   helperSize   READ helperSize   WRITE setHelperSize)
    Q_PROPERTY(int   radius       READ radius       WRITE setRadius)
    Q_PROPERTY(qreal brushOpacity READ brushOpacity WRITE setBrushOpacity)
    Q_PROPERTY(bool  changed      READ changed)

    Q_ENUMS(Mode)
    Q_ENUMS(MouseState)

public:
    explicit SketchEditor(QQuickItem *parent = nullptr);
    virtual ~SketchEditor();

    int  mode() const;
    void setMode(int mode);

    int  brushSize() const;
    void setBrushSize(int size);

    int  helperSize() const;
    void setHelperSize(int size);

    int  radius() const;
    void setRadius(int radius);

    qreal brushOpacity() const;
    void  setBrushOpacity(qreal opacity);

    bool changed() const;

    Q_INVOKABLE void openImage(QString image_file, int image_orientation);
    Q_INVOKABLE void saveImage(QString image_file);

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
    void effectedImageReady(QImage effected_image);

private slots:
    void scaleWasChanged();

signals:
    void imageOpened();
    void imageOpenFailed();

    void imageSaved(QString image_file);
    void imageSaveFailed();

    void undoAvailabilityChanged(bool available);

    void mouseEvent(int event_type, int x, int y);

    void helperImageReady(QImage helper_image);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    void SaveUndoImage();
    void ChangeImageAt(bool save_undo, int center_x, int center_y);

    static const int UNDO_DEPTH = 8;

    constexpr static const qreal IMAGE_MPIX_LIMIT = 1.0;

    bool           IsChanged;
    int            CurrentMode, BrushSize, HelperSize, GaussianRadius;
    qreal          BrushOpacity;
    QImage         LoadedImage, OriginalImage, EffectedImage, CurrentImage, BrushTemplateImage, BrushImage;
    QStack<QImage> UndoStack;
};

class SketchPreviewGenerator : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(int radius READ radius WRITE setRadius)

public:
    explicit SketchPreviewGenerator(QQuickItem *parent = nullptr);
    virtual ~SketchPreviewGenerator();

    int  radius() const;
    void setRadius(int radius);

    Q_INVOKABLE void openImage(QString image_file, int image_orientation);

    virtual void paint(QPainter *painter);

public slots:
    void sketchImageReady(QImage sketch_image);

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
    explicit SketchImageGenerator(QObject *parent = nullptr);
    virtual ~SketchImageGenerator();

    void setGaussianRadius(int radius);
    void setInput(QImage input_image);

public slots:
    void start();

signals:
    void imageReady(QImage output_image);
    void finished();

private:
    int    GaussianRadius;
    QImage InputImage;
};

#endif // SKETCHEDITOR_H
