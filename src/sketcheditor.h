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

    Q_PROPERTY(bool changed READ changed)

    Q_PROPERTY(int   mode         READ mode         WRITE setMode)
    Q_PROPERTY(int   brushSize    READ brushSize    WRITE setBrushSize)
    Q_PROPERTY(int   helperSize   READ helperSize   WRITE setHelperSize)
    Q_PROPERTY(int   radius       READ radius       WRITE setRadius)
    Q_PROPERTY(qreal brushOpacity READ brushOpacity WRITE setBrushOpacity)

    Q_ENUMS(Mode)
    Q_ENUMS(MouseState)

public:
    explicit SketchEditor(QQuickItem *parent = nullptr);
    ~SketchEditor() override = default;

    bool changed() const;

    int mode() const;
    void setMode(int mode);

    int brushSize() const;
    void setBrushSize(int size);

    int helperSize() const;
    void setHelperSize(int size);

    int radius() const;
    void setRadius(int radius);

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

public slots:
    void effectedImageReady(const QImage &effected_image);

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
    ~SketchPreviewGenerator() override = default;

    int radius() const;
    void setRadius(int radius);

    Q_INVOKABLE void openImage(const QString &image_file, int image_orientation);

    void paint(QPainter *painter) override;

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
    explicit SketchImageGenerator(QObject *parent = nullptr);
    ~SketchImageGenerator() override = default;

    void setGaussianRadius(int radius);
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
