#ifndef PIXELATEEDITOR_H
#define PIXELATEEDITOR_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStack>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtQuick/QQuickPaintedItem>

class PixelateEditor : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(int  mode               READ mode               WRITE setMode)
    Q_PROPERTY(int  helperSize         READ helperSize         WRITE setHelperSize)
    Q_PROPERTY(int  screenPixelDensity READ screenPixelDensity WRITE setScreenPixelDensity)
    Q_PROPERTY(int  pixDenom           READ pixDenom           WRITE setPixDenom)
    Q_PROPERTY(bool changed            READ changed)

    Q_ENUMS(Mode)
    Q_ENUMS(MouseState)

public:
    explicit PixelateEditor(QQuickItem *parent = 0);
    virtual ~PixelateEditor();

    int  mode() const;
    void setMode(const int &mode);

    int  helperSize() const;
    void setHelperSize(const int &size);

    int  screenPixelDensity() const;
    void setScreenPixelDensity(const int &density);

    int  pixDenom() const;
    void setPixDenom(const int &pix_denom);

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
    int            CurrentMode, HelperSize, ScreenPixelDensity, PixelDenom;
    QImage         LoadedImage, OriginalImage, EffectedImage, CurrentImage;
    QStack<QImage> UndoStack;
};

class PixelatePreviewGenerator : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(int pixDenom READ pixDenom WRITE setPixDenom)

public:
    explicit PixelatePreviewGenerator(QQuickItem *parent = 0);
    virtual ~PixelatePreviewGenerator();

    int  pixDenom() const;
    void setPixDenom(const int &pix_denom);

    Q_INVOKABLE void openImage(const QString &image_file);

    virtual void paint(QPainter *painter);

public slots:
    void pixelatedImageReady(const QImage &pixelated_image);

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
    explicit PixelateImageGenerator(QObject *parent = 0);
    virtual ~PixelateImageGenerator();

    void setPixelDenom(const int &pix_denom);
    void setInput(const QImage &input_image);

public slots:
    void start();

signals:
    void imageReady(const QImage &output_image);
    void finished();

private:
    int    PixelDenom;
    QImage InputImage;
};

#endif // PIXELATEEDITOR_H
