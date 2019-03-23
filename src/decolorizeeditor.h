#ifndef DECOLORIZEEDITOR_H
#define DECOLORIZEEDITOR_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtGui/QMouseEvent>
#include <QtGui/QImage>

#include "editor.h"

class DecolorizeEditor : public Editor
{
    Q_OBJECT

    Q_ENUMS(Mode)

public:
    explicit DecolorizeEditor(QQuickItem *parent = nullptr);
    ~DecolorizeEditor() override = default;

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

    QImage OriginalImage, EffectedImage;
};

class GrayscaleImageGenerator : public QObject
{
    Q_OBJECT

public:
    explicit GrayscaleImageGenerator(QObject *parent = nullptr);
    ~GrayscaleImageGenerator() override = default;

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
