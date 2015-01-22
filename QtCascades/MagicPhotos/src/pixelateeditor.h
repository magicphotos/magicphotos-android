#ifndef PIXELATEEDITOR_H
#define PIXELATEEDITOR_H

#include <QtCore/qmath.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStack>
#include <QtGui/QImage>

#include <bb/ImageData>
#include <bb/cascades/Image>
#include <bb/cascades/CustomControl>

class PixelateEditor : public bb::cascades::CustomControl
{
    Q_OBJECT

    Q_PROPERTY(int  mode       READ mode       WRITE setMode)
    Q_PROPERTY(int  brushSize  READ brushSize  WRITE setBrushSize)
    Q_PROPERTY(int  helperSize READ helperSize WRITE setHelperSize)
    Q_PROPERTY(int  pixDenom   READ pixDenom   WRITE setPixDenom)
    Q_PROPERTY(bool changed    READ changed)

    Q_ENUMS(Mode)

public:
    explicit PixelateEditor();
    virtual ~PixelateEditor();

    int  mode() const;
    void setMode(const int &mode);

    int  brushSize() const;
    void setBrushSize(const int &size);

    int  helperSize() const;
    void setHelperSize(const int &size);

    int  pixDenom() const;
    void setPixDenom(const int &pix_denom);

    bool changed() const;

    Q_INVOKABLE void openImage(const QString &image_file);
    Q_INVOKABLE void saveImage(const QString &image_file);

    Q_INVOKABLE void changeImageAt(bool save_undo, int center_x, int center_y, qreal zoom_level);

    Q_INVOKABLE void undo();

    enum Mode {
        ModeScroll,
        ModeOriginal,
        ModeEffected
    };

public slots:
    void effectedImageReady(const QImage &effected_image);

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
    void RepaintHelper(int center_x, int center_y, qreal zoom_level);

    static const int UNDO_DEPTH = 4;

    static const qreal IMAGE_MPIX_LIMIT = 1.0;

    bool           IsChanged;
    int            CurrentMode, BrushSize, HelperSize, PixelDenom;
    QImage         LoadedImage, OriginalImage, EffectedImage, CurrentImage;
    QStack<QImage> UndoStack;
    bb::ImageData  CurrentImageData;
};

class PixelatePreviewGenerator : public bb::cascades::CustomControl
{
    Q_OBJECT

    Q_PROPERTY(int pixDenom READ pixDenom WRITE setPixDenom)

public:
    explicit PixelatePreviewGenerator();
    virtual ~PixelatePreviewGenerator();

    int  pixDenom() const;
    void setPixDenom(const int &pix_denom);

    Q_INVOKABLE void openImage(const QString &image_file);

public slots:
    void pixelatedImageReady(const QImage &pixelated_image);

signals:
    void imageOpened();
    void imageOpenFailed();

    void generationStarted();
    void generationFinished();

    void needRepaint(const bb::cascades::Image &image);

private:
    void Repaint();

    static const qreal IMAGE_MPIX_LIMIT = 0.5;

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
