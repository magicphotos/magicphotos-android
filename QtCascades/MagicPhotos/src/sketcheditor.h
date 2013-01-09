#ifndef SKETCHEDITOR_H
#define SKETCHEDITOR_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStack>
#include <QtGui/QImage>
#include <bb/ImageData>
#include <bb/cascades/Image>
#include <bb/cascades/CustomControl>

class SketchEditor : public bb::cascades::CustomControl
{
    Q_OBJECT

    Q_PROPERTY(int  mode    READ mode   WRITE setMode)
    Q_PROPERTY(int  radius  READ radius WRITE setRadius)
    Q_PROPERTY(bool changed READ changed)

    Q_ENUMS(Mode)

public:
    explicit SketchEditor();
    virtual ~SketchEditor();

    int  mode() const;
    void setMode(const int &mode);

    int  radius() const;
    void setRadius(const int &radius);

    bool changed() const;

    Q_INVOKABLE void openImage(const QString &image_file);
    Q_INVOKABLE void saveImage(const QString &image_file);

    Q_INVOKABLE void changeImageAt(bool save_undo, int center_x, int center_y, double zoom_level);

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
    void RepaintHelper(int center_x, int center_y, double zoom_level);

    static const int UNDO_DEPTH  = 4,
                     BRUSH_SIZE  = 32,
                     HELPER_SIZE = 192;

    static const qreal IMAGE_MPIX_LIMIT = 1.0;

    bool           IsChanged;
    int            CurrentMode, GaussianRadius;
    QImage         LoadedImage, OriginalImage, EffectedImage, CurrentImage;
    QStack<QImage> UndoStack;
    bb::ImageData  CurrentImageData;
};

class SketchPreviewGenerator : public bb::cascades::CustomControl
{
    Q_OBJECT

    Q_PROPERTY(int radius READ radius WRITE setRadius)

public:
    explicit SketchPreviewGenerator();
    virtual ~SketchPreviewGenerator();

    int  radius() const;
    void setRadius(const int &radius);

    Q_INVOKABLE void openImage(const QString &image_file);

public slots:
    void sketchImageReady(const QImage &sketch_image);

signals:
    void imageOpened();
    void imageOpenFailed();

    void needRepaint(const bb::cascades::Image &image);

private:
    void Repaint();

    static const qreal IMAGE_MPIX_LIMIT = 0.5;

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
