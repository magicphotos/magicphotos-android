#ifndef PIXELATEEDITOR_H
#define PIXELATEEDITOR_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStack>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtQuick/QQuickPaintedItem>

class PixelateEditor : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(bool changed READ changed)

    Q_PROPERTY(int   mode         READ mode         WRITE setMode)
    Q_PROPERTY(int   brushSize    READ brushSize    WRITE setBrushSize)
    Q_PROPERTY(int   helperSize   READ helperSize   WRITE setHelperSize)
    Q_PROPERTY(int   pixDenom     READ pixDenom     WRITE setPixDenom)
    Q_PROPERTY(qreal brushOpacity READ brushOpacity WRITE setBrushOpacity)

    Q_ENUMS(Mode)
    Q_ENUMS(MouseState)

public:
    explicit PixelateEditor(QQuickItem *parent = nullptr);
    virtual ~PixelateEditor();

    bool changed() const;

    int mode() const;
    void setMode(int mode);

    int brushSize() const;
    void setBrushSize(int size);

    int helperSize() const;
    void setHelperSize(int size);

    int pixDenom() const;
    void setPixDenom(int pix_denom);

    qreal brushOpacity() const;
    void setBrushOpacity(qreal opacity);

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

    void imageSaved(QString imageFile);
    void imageSaveFailed();

    void undoAvailabilityChanged(bool available);

    void mouseEvent(int eventType, int x, int y);

    void helperImageReady(QImage helperImage);

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
    int            CurrentMode, BrushSize, HelperSize, PixelDenom;
    qreal          BrushOpacity;
    QImage         LoadedImage, OriginalImage, EffectedImage, CurrentImage, BrushTemplateImage, BrushImage;
    QStack<QImage> UndoStack;
};

class PixelatePreviewGenerator : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(int pixDenom READ pixDenom WRITE setPixDenom)

public:
    explicit PixelatePreviewGenerator(QQuickItem *parent = nullptr);
    virtual ~PixelatePreviewGenerator();

    int pixDenom() const;
    void setPixDenom(int pix_denom);

    Q_INVOKABLE void openImage(QString image_file, int image_orientation);

    virtual void paint(QPainter *painter);

public slots:
    void pixelatedImageReady(QImage pixelated_image);

signals:
    void imageOpened();
    void imageOpenFailed();

    void generationStarted();
    void generationFinished();

private:
    void StartPixelateGenerator();

    constexpr static const qreal IMAGE_MPIX_LIMIT = 0.2;

    bool   PixelateGeneratorRunning, RestartPixelateGenerator;
    int    PixelDenom;
    QImage LoadedImage, PixelatedImage;
};

class PixelateImageGenerator : public QObject
{
    Q_OBJECT

public:
    explicit PixelateImageGenerator(QObject *parent = nullptr);
    virtual ~PixelateImageGenerator();

    void setPixelDenom(int pix_denom);
    void setInput(QImage input_image);

public slots:
    void start();

signals:
    void imageReady(QImage output_image);
    void finished();

private:
    int    PixelDenom;
    QImage InputImage;
};

#endif // PIXELATEEDITOR_H
