#ifndef CARTOONEDITOR_H
#define CARTOONEDITOR_H

#include <QtCore/QObject>
#include <QtGui/QMouseEvent>
#include <QtGui/QImage>

#include "effecteditor.h"
#include "previewgenerator.h"

class CartoonEditor : public EffectEditor
{
    Q_OBJECT

    Q_PROPERTY(int radius    READ radius    WRITE setRadius)
    Q_PROPERTY(int threshold READ threshold WRITE setThreshold)

public:
    explicit CartoonEditor(QQuickItem *parent = nullptr);
    ~CartoonEditor() override = default;

    int radius() const;
    void setRadius(int radius);

    int threshold() const;
    void setThreshold(int threshold);

protected:
    void processOpenedImage() override;

private:
    int GaussianRadius, CartoonThreshold;
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
