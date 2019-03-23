#ifndef CARTOONEDITOR_H
#define CARTOONEDITOR_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtGui/QMouseEvent>
#include <QtGui/QImage>

#include "editor.h"
#include "previewgenerator.h"

class CartoonEditor : public Editor
{
    Q_OBJECT

    Q_PROPERTY(int radius    READ radius    WRITE setRadius)
    Q_PROPERTY(int threshold READ threshold WRITE setThreshold)

    Q_ENUMS(Mode)

public:
    explicit CartoonEditor(QQuickItem *parent = nullptr);
    ~CartoonEditor() override = default;

    int radius() const;
    void setRadius(int radius);

    int threshold() const;
    void setThreshold(int threshold);

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

    int    GaussianRadius, CartoonThreshold;
    QImage OriginalImage, EffectedImage;
};

class CartoonPreviewGenerator : public PreviewGenerator
{
    Q_OBJECT

    Q_PROPERTY(int radius    READ radius    WRITE setRadius)
    Q_PROPERTY(int threshold READ threshold WRITE setThreshold)

public:
    explicit CartoonPreviewGenerator(QQuickItem *parent = nullptr);
    ~CartoonPreviewGenerator() override = default;

    int radius() const;
    void setRadius(int radius);

    int threshold() const;
    void setThreshold(int threshold);

protected:
    void StartImageGenerator() override;

private:
    int GaussianRadius, CartoonThreshold;
};

class CartoonImageGenerator : public QObject
{
    Q_OBJECT

public:
    explicit CartoonImageGenerator(QObject *parent = nullptr);
    ~CartoonImageGenerator() override = default;

    void setGaussianRadius(int radius);
    void setCartoonThreshold(int threshold);
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
