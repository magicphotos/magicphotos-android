#ifndef PIXELATEEDITOR_H
#define PIXELATEEDITOR_H

#include <QtCore/QObject>
#include <QtGui/QMouseEvent>
#include <QtGui/QImage>

#include "effecteditor.h"
#include "previewgenerator.h"

class PixelateEditor : public EffectEditor
{
    Q_OBJECT

    Q_PROPERTY(int pixDenom READ pixDenom WRITE setPixDenom)

public:
    explicit PixelateEditor(QQuickItem *parent = nullptr);

    PixelateEditor(const PixelateEditor &) = delete;
    PixelateEditor(PixelateEditor &&) noexcept = delete;

    PixelateEditor &operator=(const PixelateEditor &) = delete;
    PixelateEditor &operator=(PixelateEditor &&) noexcept = delete;

    ~PixelateEditor() noexcept override = default;

    int pixDenom() const;
    void setPixDenom(int pix_denom);

protected:
    void processOpenedImage() override;

private:
    int PixelDenom;
};

class PixelatePreviewGenerator : public PreviewGenerator
{
    Q_OBJECT

    Q_PROPERTY(int pixDenom READ pixDenom WRITE setPixDenom)

public:
    explicit PixelatePreviewGenerator(QQuickItem *parent = nullptr);

    PixelatePreviewGenerator(const PixelatePreviewGenerator &) = delete;
    PixelatePreviewGenerator(PixelatePreviewGenerator &&) noexcept = delete;

    PixelatePreviewGenerator &operator=(const PixelatePreviewGenerator &) = delete;
    PixelatePreviewGenerator &operator=(PixelatePreviewGenerator &&) noexcept = delete;

    ~PixelatePreviewGenerator() noexcept override = default;

    int pixDenom() const;
    void setPixDenom(int pix_denom);

protected:
    void StartImageGenerator() override;

private:
    int PixelDenom;
};

class PixelateImageGenerator : public QObject
{
    Q_OBJECT

public:
    explicit PixelateImageGenerator(QObject *parent = nullptr);

    PixelateImageGenerator(const PixelateImageGenerator &) = delete;
    PixelateImageGenerator(PixelateImageGenerator &&) noexcept = delete;

    PixelateImageGenerator &operator=(const PixelateImageGenerator &) = delete;
    PixelateImageGenerator &operator=(PixelateImageGenerator &&) noexcept = delete;

    ~PixelateImageGenerator() noexcept override = default;

    void setPixelDenom(int pix_denom);
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
