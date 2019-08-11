#ifndef DECOLORIZEEDITOR_H
#define DECOLORIZEEDITOR_H

#include <QtCore/QObject>
#include <QtGui/QMouseEvent>
#include <QtGui/QImage>

#include "effecteditor.h"

class DecolorizeEditor : public EffectEditor
{
    Q_OBJECT

public:
    explicit DecolorizeEditor(QQuickItem *parent = nullptr);

    DecolorizeEditor(const DecolorizeEditor &) = delete;
    DecolorizeEditor(DecolorizeEditor &&) noexcept = delete;

    DecolorizeEditor &operator=(const DecolorizeEditor &) = delete;
    DecolorizeEditor &operator=(DecolorizeEditor &&) noexcept = delete;

    ~DecolorizeEditor() noexcept override = default;

protected:
    void processOpenedImage() override;
};

class GrayscaleImageGenerator : public QObject
{
    Q_OBJECT

public:
    explicit GrayscaleImageGenerator(QObject *parent = nullptr);

    GrayscaleImageGenerator(const GrayscaleImageGenerator &) = delete;
    GrayscaleImageGenerator(GrayscaleImageGenerator &&) noexcept = delete;

    GrayscaleImageGenerator &operator=(const GrayscaleImageGenerator &) = delete;
    GrayscaleImageGenerator &operator=(GrayscaleImageGenerator &&) noexcept = delete;

    ~GrayscaleImageGenerator() noexcept override = default;

    void setInput(const QImage &input_image);

public slots:
    void start();

signals:
    void imageReady(const QImage &outputImage);
    void finished();

private:
    QImage InputImage;
};

#endif // DECOLORIZEEDITOR_H
