#ifndef PIXELATEEDITOR_H
#define PIXELATEEDITOR_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtGui/QMouseEvent>
#include <QtGui/QImage>

#include "editor.h"
#include "previewgenerator.h"

class PixelateEditor : public Editor
{
    Q_OBJECT

    Q_PROPERTY(int pixDenom READ pixDenom WRITE setPixDenom)

    Q_ENUMS(Mode)

public:
    explicit PixelateEditor(QQuickItem *parent = nullptr);
    ~PixelateEditor() override = default;

    int pixDenom() const;
    void setPixDenom(int pix_denom);

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

    int    PixelDenom;
    QImage OriginalImage, EffectedImage;
};

class PixelatePreviewGenerator : public PreviewGenerator
{
    Q_OBJECT

    Q_PROPERTY(int pixDenom READ pixDenom WRITE setPixDenom)

public:
    explicit PixelatePreviewGenerator(QQuickItem *parent = nullptr);
    ~PixelatePreviewGenerator() override = default;

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
    ~PixelateImageGenerator() override = default;

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
