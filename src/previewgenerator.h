#ifndef PREVIEWGENERATOR_H
#define PREVIEWGENERATOR_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtQuick/QQuickPaintedItem>

class PreviewGenerator : public QQuickPaintedItem
{
    Q_OBJECT

public:
    explicit PreviewGenerator(QQuickItem *parent = nullptr);

    PreviewGenerator(const PreviewGenerator &) = delete;
    PreviewGenerator(PreviewGenerator &&) noexcept = delete;

    PreviewGenerator &operator=(const PreviewGenerator &) = delete;
    PreviewGenerator &operator=(PreviewGenerator &&) noexcept = delete;

    ~PreviewGenerator() noexcept override = default;

    Q_INVOKABLE void openImage(const QString &image_file, int image_orientation);

    void paint(QPainter *painter) override;

protected slots:
    void setEffectedImage(const QImage &image);

signals:
    void imageOpened();
    void imageOpenFailed();
    void generationStarted();
    void generationFinished();

protected:
    virtual void StartImageGenerator() = 0;

    bool   ImageGeneratorRunning, RestartImageGenerator;
    QImage LoadedImage;

private:
    static constexpr qreal IMAGE_MPIX_LIMIT = 0.2;

    QImage EffectedImage;
};

#endif // PREVIEWGENERATOR_H
