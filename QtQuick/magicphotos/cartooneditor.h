#ifndef CARTOONEDITOR_H
#define CARTOONEDITOR_H

#include <QObject>
#include <QString>
#include <QStack>
#include <QImage>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QDeclarativeItem>

class CartoonEditor : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(int  mode       READ mode       WRITE setMode)
    Q_PROPERTY(int  helperSize READ helperSize WRITE setHelperSize)
    Q_PROPERTY(int  radius     READ radius     WRITE setRadius)
    Q_PROPERTY(int  threshold  READ threshold  WRITE setThreshold)
    Q_PROPERTY(bool changed    READ changed)

    Q_ENUMS(Mode)
    Q_ENUMS(MouseState)

public:
    explicit CartoonEditor(QDeclarativeItem *parent = 0);
    virtual ~CartoonEditor();

    int  mode() const;
    void setMode(const int &mode);

    int  helperSize() const;
    void setHelperSize(const int &size);

    int  radius() const;
    void setRadius(const int &radius);

    int  threshold() const;
    void setThreshold(const int &threshold);

    bool changed() const;

    Q_INVOKABLE void openImage(const QString &image_url);
    Q_INVOKABLE void saveImage(const QString &image_url);

    Q_INVOKABLE void undo();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget*);

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
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    void SaveUndoImage();
    void ChangeImageAt(bool save_undo, int center_x, int center_y);

    static const int UNDO_DEPTH  = 4,
                     BRUSH_SIZE  = 16;

    static const qreal IMAGE_MPIX_LIMIT = 1.0;

    bool           IsChanged;
    int            CurrentMode, HelperSize, GaussianRadius, CartoonThreshold;
    QImage         LoadedImage, OriginalImage, EffectedImage, CurrentImage;
    QStack<QImage> UndoStack;
};

class CartoonPreviewGenerator : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(int radius    READ radius    WRITE setRadius)
    Q_PROPERTY(int threshold READ threshold WRITE setThreshold)

public:
    explicit CartoonPreviewGenerator(QDeclarativeItem *parent = 0);
    virtual ~CartoonPreviewGenerator();

    int  radius() const;
    void setRadius(const int &radius);

    int  threshold() const;
    void setThreshold(const int &threshold);

    Q_INVOKABLE void openImage(const QString &image_url);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*);

public slots:
    void cartoonImageReady(const QImage &cartoon_image);

signals:
    void imageOpened();
    void imageOpenFailed();

    void generationStarted();
    void generationFinished();

private:
    void StartCartoonGenerator();

    static const qreal IMAGE_MPIX_LIMIT = 0.2;

    bool   CartoonGeneratorRunning, RestartCartoonGenerator;
    int    GaussianRadius, CartoonThreshold;
    QImage LoadedImage, CartoonImage;
};

class CartoonImageGenerator : public QObject
{
    Q_OBJECT

public:
    explicit CartoonImageGenerator(QObject *parent = 0);
    virtual ~CartoonImageGenerator();

    void setGaussianRadius(const int &radius);
    void setCartoonThreshold(const int &threshold);
    void setInput(const QImage &input_image);

public slots:
    void start();

signals:
    void imageReady(const QImage &output_image);
    void finished();

private:
    int    GaussianRadius, CartoonThreshold;
    QImage InputImage;
};

#endif // CARTOONEDITOR_H
