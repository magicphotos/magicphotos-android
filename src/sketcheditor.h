#ifndef SKETCHEDITOR_H
#define SKETCHEDITOR_H

#include <QtCore/QObject>
#include <QtGui/QMouseEvent>
#include <QtGui/QImage>

#include "effecteditor.h"
#include "previewgenerator.h"

class SketchEditor : public EffectEditor
{
    Q_OBJECT

    Q_PROPERTY(int radius READ radius WRITE setRadius)

public:
    explicit SketchEditor(QQuickItem *parent = nullptr);

    SketchEditor(const SketchEditor &) = delete;
    SketchEditor(SketchEditor &&) noexcept = delete;

    SketchEditor &operator=(const SketchEditor &) = delete;
    SketchEditor &operator=(SketchEditor &&) noexcept = delete;

    ~SketchEditor() noexcept override = default;

    int radius() const;
    void setRadius(int radius);

protected:
    void processOpenedImage() override;

private:
    int GaussianRadius;
};

class SketchPreviewGenerator : public PreviewGenerator
{
    Q_OBJECT

    Q_PROPERTY(int radius READ radius WRITE setRadius)

public:
    explicit SketchPreviewGenerator(QQuickItem *parent = nullptr);

    SketchPreviewGenerator(const SketchPreviewGenerator &) = delete;
    SketchPreviewGenerator(SketchPreviewGenerator &&) noexcept = delete;

    SketchPreviewGenerator &operator=(const SketchPreviewGenerator &) = delete;
    SketchPreviewGenerator &operator=(SketchPreviewGenerator &&) noexcept = delete;

    ~SketchPreviewGenerator() noexcept override = default;

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

    SketchImageGenerator(const SketchImageGenerator &) = delete;
    SketchImageGenerator(SketchImageGenerator &&) noexcept = delete;

    SketchImageGenerator &operator=(const SketchImageGenerator &) = delete;
    SketchImageGenerator &operator=(SketchImageGenerator &&) noexcept = delete;

    ~SketchImageGenerator() noexcept override = default;

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
