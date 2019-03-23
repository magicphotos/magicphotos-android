#ifndef SKETCHEDITOR_H
#define SKETCHEDITOR_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtGui/QMouseEvent>
#include <QtGui/QImage>

#include "editor.h"
#include "previewgenerator.h"

class SketchEditor : public Editor
{
    Q_OBJECT

    Q_PROPERTY(int radius READ radius WRITE setRadius)

    Q_ENUMS(Mode)

public:
    explicit SketchEditor(QQuickItem *parent = nullptr);
    ~SketchEditor() override = default;

    int radius() const;
    void setRadius(int radius);

    enum Mode {
        ModeScroll,
        ModeOriginal,
        ModeEffected
    };

private slots:
    void effectedImageReady(const QImage &effected_image);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void processOpenedImage() override;

private:
    void ChangeImageAt(bool save_undo, int center_x, int center_y);

    int    GaussianRadius;
    QImage OriginalImage, EffectedImage;
};

class SketchPreviewGenerator : public PreviewGenerator
{
    Q_OBJECT

    Q_PROPERTY(int radius READ radius WRITE setRadius)

public:
    explicit SketchPreviewGenerator(QQuickItem *parent = nullptr);
    ~SketchPreviewGenerator() override = default;

    int radius() const;
    void setRadius(int radius);

protected:
    void StartImageGenerator() override;

private:
    int GaussianRadius;
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
