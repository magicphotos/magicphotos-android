#ifndef DECOLORIZEEDITOR_H
#define DECOLORIZEEDITOR_H

#include <QObject>
#include <QString>
#include <QStack>
#include <QImage>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QDeclarativeItem>

class DecolorizeEditor : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(int  mode    READ mode    WRITE setMode)
    Q_PROPERTY(bool changed READ changed)

    Q_ENUMS(Mode)

public:
    explicit DecolorizeEditor(QDeclarativeItem *parent = 0);
    virtual ~DecolorizeEditor();

    int  mode() const;
    void setMode(const int &mode);

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

public slots:
    void effectedImageReady(const QImage &effected_image);

signals:
    void imageOpened();
    void imageOpenFailed();

    void imageSaved();
    void imageSaveFailed();

    void undoAvailabilityChanged(bool available);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
    void SaveUndoImage();
    void ChangeImageAt(bool save_undo, int center_x, int center_y);

    static const int UNDO_DEPTH  = 4,
                     BRUSH_SIZE  = 16,
                     HELPER_SIZE = 192;

    static const qreal IMAGE_MPIX_LIMIT = 1.0;

    bool           IsChanged;
    int            CurrentMode;
    QImage         LoadedImage, OriginalImage, EffectedImage, CurrentImage;
    QStack<QImage> UndoStack;
};

class GrayscaleImageGenerator : public QObject
{
    Q_OBJECT

public:
    explicit GrayscaleImageGenerator(QObject *parent = 0);
    virtual ~GrayscaleImageGenerator();

    void setInput(const QImage &input_image);

public slots:
    void start();

signals:
    void imageReady(const QImage &output_image);
    void finished();

private:
    QImage InputImage;
};

#endif // DECOLORIZEEDITOR_H
