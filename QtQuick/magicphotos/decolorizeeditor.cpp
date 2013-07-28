#include <qmath.h>
#include <QFileInfo>
#include <QDir>
#include <QThread>
#include <QImageReader>
#include <QPainter>

#include "decolorizeeditor.h"

DecolorizeEditor::DecolorizeEditor(QDeclarativeItem *parent) : QDeclarativeItem(parent)
{
    IsChanged   = false;
    CurrentMode = ModeScroll;
    HelperSize  = 0;

    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);

    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);
    setFlag(QGraphicsItem::ItemHasNoContents,           false);
}

DecolorizeEditor::~DecolorizeEditor()
{
}

int DecolorizeEditor::mode() const
{
    return CurrentMode;
}

void DecolorizeEditor::setMode(const int &mode)
{
    CurrentMode = mode;
}

int DecolorizeEditor::helperSize() const
{
    return HelperSize;
}

void DecolorizeEditor::setHelperSize(const int &size)
{
    HelperSize = size;
}

bool DecolorizeEditor::changed() const
{
    return IsChanged;
}

void DecolorizeEditor::openImage(const QString &image_url)
{
    QString image_file = QUrl(image_url).toLocalFile();

    if (!image_file.isNull()) {
        QImageReader reader(image_file);

        if (reader.canRead()) {
            QSize size = reader.size();

            if (size.width() * size.height() > IMAGE_MPIX_LIMIT * 1000000.0) {
                qreal factor = qSqrt((size.width() * size.height()) / (IMAGE_MPIX_LIMIT * 1000000.0));

                size.setWidth(size.width()   / factor);
                size.setHeight(size.height() / factor);

                reader.setScaledSize(size);
            }

            LoadedImage = reader.read();

            if (!LoadedImage.isNull()) {
                LoadedImage = LoadedImage.convertToFormat(QImage::Format_RGB16);

                if (!LoadedImage.isNull()) {
                    QThread                 *thread    = new QThread();
                    GrayscaleImageGenerator *generator = new GrayscaleImageGenerator();

                    generator->moveToThread(thread);

                    QObject::connect(thread,    SIGNAL(started()),                  generator, SLOT(start()));
                    QObject::connect(thread,    SIGNAL(finished()),                 thread,    SLOT(deleteLater()));
                    QObject::connect(generator, SIGNAL(imageReady(const QImage &)), this,      SLOT(effectedImageReady(const QImage &)));
                    QObject::connect(generator, SIGNAL(finished()),                 thread,    SLOT(quit()));
                    QObject::connect(generator, SIGNAL(finished()),                 generator, SLOT(deleteLater()));

                    generator->setInput(LoadedImage);

                    thread->start();
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

void DecolorizeEditor::saveImage(const QString &image_url)
{
    QString file_name = QUrl(image_url).toLocalFile();

    if (!file_name.isNull()) {
        if (!CurrentImage.isNull()) {
            if (QFileInfo(file_name).suffix().compare("png", Qt::CaseInsensitive) != 0 &&
                QFileInfo(file_name).suffix().compare("jpg", Qt::CaseInsensitive) != 0 &&
                QFileInfo(file_name).suffix().compare("bmp", Qt::CaseInsensitive) != 0) {
                file_name = file_name + ".jpg";
            }

            if (CurrentImage.convertToFormat(QImage::Format_ARGB32).save(file_name)) {
                IsChanged = false;

                emit imageSaved();
            } else {
                emit imageSaveFailed();
            }
        } else {
            emit imageSaveFailed();
        }
    } else {
        emit imageSaveFailed();
    }
}

void DecolorizeEditor::undo()
{
    if (UndoStack.size() > 0) {
        CurrentImage = UndoStack.pop();

        if (UndoStack.size() == 0) {
            emit undoAvailabilityChanged(false);
        }

        IsChanged = true;

        update();
    }
}

void DecolorizeEditor::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget*)
{
    qreal scale = 1.0;

    if (CurrentImage.width() != 0) {
        scale = width() / CurrentImage.width();
    }

    bool antialiasing = painter->testRenderHint(QPainter::Antialiasing);

    if (smooth()) {
        painter->setRenderHint(QPainter::Antialiasing, true);
    }

    QRectF source_rect(option->exposedRect.left()   / scale,
                       option->exposedRect.top()    / scale,
                       option->exposedRect.width()  / scale,
                       option->exposedRect.height() / scale);

    painter->drawImage(option->exposedRect, CurrentImage.copy(source_rect.toRect()));

    painter->setRenderHint(QPainter::Antialiasing, antialiasing);
}

void DecolorizeEditor::effectedImageReady(const QImage &effected_image)
{
    OriginalImage = LoadedImage;
    EffectedImage = effected_image;
    CurrentImage  = EffectedImage;

    LoadedImage = QImage();

    UndoStack.clear();

    IsChanged = true;

    setImplicitWidth(CurrentImage.width());
    setImplicitHeight(CurrentImage.height());

    update();

    emit undoAvailabilityChanged(false);
    emit imageOpened();
}

void DecolorizeEditor::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        ChangeImageAt(true, event->pos().x(), event->pos().y());

        emit mouseEvent(MousePressed, event->pos().x(), event->pos().y());
    }
}

void DecolorizeEditor::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        ChangeImageAt(false, event->pos().x(), event->pos().y());

        emit mouseEvent(MouseMoved, event->pos().x(), event->pos().y());
    }
}

void DecolorizeEditor::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (CurrentMode == ModeOriginal || CurrentMode == ModeEffected) {
        emit mouseEvent(MouseReleased, event->pos().x(), event->pos().y());
    }
}

void DecolorizeEditor::SaveUndoImage()
{
    UndoStack.push(CurrentImage);

    if (UndoStack.size() > UNDO_DEPTH) {
        for (int i = 0; i < UndoStack.size() - UNDO_DEPTH; i++) {
            UndoStack.remove(0);
        }
    }

    emit undoAvailabilityChanged(true);
}

void DecolorizeEditor::ChangeImageAt(bool save_undo, int center_x, int center_y)
{
    if (CurrentMode != ModeScroll) {
        if (save_undo) {
            SaveUndoImage();
        }

        qreal scale = 1.0;

        if (CurrentImage.width() != 0) {
            scale = width() / CurrentImage.width();
        }

        int img_center_x = center_x   / scale;
        int img_center_y = center_y   / scale;
        int radius       = BRUSH_SIZE / scale;

        for (int x = img_center_x - radius; x <= img_center_x + radius; x++) {
            for (int y = img_center_y - radius; y <= img_center_y + radius; y++) {
                if (x >= 0 && x < CurrentImage.width() && y >= 0 && y < CurrentImage.height() && qSqrt(qPow(x - img_center_x, 2) + qPow(y - img_center_y, 2)) <= radius) {
                    if (CurrentMode == ModeOriginal) {
                        CurrentImage.setPixel(x, y, OriginalImage.pixel(x, y));
                    } else {
                        CurrentImage.setPixel(x, y, EffectedImage.pixel(x, y));
                    }
                }
            }
        }

        IsChanged = true;

        update(center_x - BRUSH_SIZE, center_y - BRUSH_SIZE, BRUSH_SIZE * 2, BRUSH_SIZE * 2);

        QImage helper_image = CurrentImage.copy(img_center_x - (HelperSize / scale) / 2,
                                                img_center_y - (HelperSize / scale) / 2,
                                                HelperSize / scale,
                                                HelperSize / scale).scaledToWidth(HelperSize);

        emit helperImageReady(helper_image);
    }
}

GrayscaleImageGenerator::GrayscaleImageGenerator(QObject *parent) : QObject(parent)
{
}

GrayscaleImageGenerator::~GrayscaleImageGenerator()
{
}

void GrayscaleImageGenerator::setInput(const QImage &input_image)
{
    InputImage = input_image;
}

void GrayscaleImageGenerator::start()
{
    QImage grayscale_image = InputImage;

    for (int x = 0; x < grayscale_image.width(); x++) {
        for (int y = 0; y < grayscale_image.height(); y++) {
            int gray  = qGray(grayscale_image.pixel(x, y));
            int alpha = qAlpha(grayscale_image.pixel(x, y));

            grayscale_image.setPixel(x, y, qRgba(gray, gray, gray, alpha));
        }
    }

    emit imageReady(grayscale_image);
    emit finished();
}
