#include <QtCore/QtMath>
#include <QtCore/QSize>
#include <QtCore/QPointF>
#include <QtGui/QTransform>
#include <QtGui/QImageReader>

#include "previewgenerator.h"

PreviewGenerator::PreviewGenerator(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    ImageGeneratorRunning = false;
    RestartImageGenerator = false;

    setFlag(QQuickItem::ItemHasContents, true);
}

void PreviewGenerator::openImage(const QString &image_file, int image_orientation)
{
    if (!image_file.isNull()) {
        QImageReader reader(image_file);

        if (reader.canRead()) {
            QSize size = reader.size();

            if (!size.isEmpty()) {
                if (static_cast<qreal>(size.width()) * static_cast<qreal>(size.height()) > IMAGE_MPIX_LIMIT * 1000000.0) {
                    qreal scale = qSqrt((static_cast<qreal>(size.width()) * static_cast<qreal>(size.height())) / (IMAGE_MPIX_LIMIT * 1000000.0));

                    size.setWidth(qFloor(size.width()   / scale));
                    size.setHeight(qFloor(size.height() / scale));
                }

                if (!size.isEmpty()) {
                    reader.setScaledSize(size);

                    LoadedImage = reader.read();

                    if (!LoadedImage.isNull()) {
                        if (image_orientation == 90) {
                            QTransform transform;

                            transform.rotate(90);

                            LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
                        } else if (image_orientation == 180) {
                            QTransform transform;

                            transform.rotate(180);

                            LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.width(), LoadedImage.height());
                        } else if (image_orientation == 270) {
                            QTransform transform;

                            transform.rotate(270);

                            LoadedImage = LoadedImage.transformed(transform).scaled(LoadedImage.height(), LoadedImage.width());
                        }

                        LoadedImage = LoadedImage.convertToFormat(QImage::Format_RGB32);

                        if (!LoadedImage.isNull()) {
                            emit imageOpened();

                            if (ImageGeneratorRunning) {
                                RestartImageGenerator = true;
                            } else {
                                StartImageGenerator();
                            }
                        } else {
                            emit imageOpenFailed();
                        }
                    } else {
                        emit imageOpenFailed();
                    }
                } else {
                    emit imageOpenFailed();
                }
            } else {
                emit imageOpenFailed();
            }
        } else {
            emit imageOpenFailed();
        }
    } else {
        emit imageOpenFailed();
    }
}

void PreviewGenerator::paint(QPainter *painter)
{
    if (!EffectedImage.isNull()) {
        QImage image = EffectedImage.scaled(QSize(qFloor(width()), qFloor(height())), Qt::KeepAspectRatio,
                                            smooth() ? Qt::SmoothTransformation : Qt::FastTransformation);

        painter->drawImage(QPointF((width()  - image.width())  / 2,
                                   (height() - image.height()) / 2), image);
    }
}

void PreviewGenerator::setEffectedImage(const QImage &image)
{
    ImageGeneratorRunning = false;
    EffectedImage         = image;

    setImplicitWidth(EffectedImage.width());
    setImplicitHeight(EffectedImage.height());

    update();

    emit generationFinished();

    if (RestartImageGenerator) {
        StartImageGenerator();

        RestartImageGenerator = false;
    }
}
