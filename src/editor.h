#ifndef EDITOR_H
#define EDITOR_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStack>
#include <QtGui/QMouseEvent>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtQuick/QQuickPaintedItem>

class Editor : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(bool changed READ changed)

    Q_PROPERTY(int   mode         READ mode         WRITE setMode)
    Q_PROPERTY(int   brushSize    READ brushSize    WRITE setBrushSize)
    Q_PROPERTY(int   helperSize   READ helperSize   WRITE setHelperSize)
    Q_PROPERTY(qreal brushOpacity READ brushOpacity WRITE setBrushOpacity)

    Q_ENUMS(Mode)
    Q_ENUMS(MouseState)

public:
    explicit Editor(QQuickItem *parent = nullptr);
    ~Editor() override = default;

    bool changed() const;

    int mode() const;
    void setMode(int mode);

    int brushSize() const;
    void setBrushSize(int size);

    int helperSize() const;
    void setHelperSize(int size);

    qreal brushOpacity() const;
    void setBrushOpacity(qreal opacity);

    Q_INVOKABLE void openImage(const QString &image_file, int image_orientation);
    Q_INVOKABLE void saveImage(const QString &image_file);

    Q_INVOKABLE void undo();

    void paint(QPainter *painter) override;

    enum Mode {
        ModeScroll
    };

    enum MouseState {
        MousePressed,
        MouseMoved,
        MouseReleased
    };

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
    void mousePressEvent(QMouseEvent *event) override = 0;
    void mouseMoveEvent(QMouseEvent *event) override = 0;
    void mouseReleaseEvent(QMouseEvent *event) override = 0;

    virtual void processOpenedImage() = 0;

    void SaveUndoImage();

    bool   IsChanged;
    int    CurrentMode, BrushSize, HelperSize;
    qreal  BrushOpacity;
    QImage LoadedImage, CurrentImage, BrushTemplateImage, BrushImage;

private:
    constexpr static const int   UNDO_DEPTH       = 8;
    constexpr static const qreal IMAGE_MPIX_LIMIT = 1.0;

    QStack<QImage> UndoStack;
};

#endif // EDITOR_H
