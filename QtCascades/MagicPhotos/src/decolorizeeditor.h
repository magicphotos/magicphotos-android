#ifndef DECOLORIZEEDITOR_H
#define DECOLORIZEEDITOR_H

#include <QtCore/qmath.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QPair>
#include <QtCore/QList>
#include <QtCore/QStack>
#include <QtCore/QMap>
#include <QtGui/QImage>

#include <bb/cascades/ImageView>
#include <bb/cascades/CustomControl>

class DecolorizeEditor : public bb::cascades::CustomControl
{
    Q_OBJECT

    Q_PROPERTY(int                      mode            READ mode            WRITE setMode)
    Q_PROPERTY(int                      brushSize       READ brushSize       WRITE setBrushSize)
    Q_PROPERTY(int                      helperSize      READ helperSize      WRITE setHelperSize)
    Q_PROPERTY(qreal                    brushOpacity    READ brushOpacity    WRITE setBrushOpacity)
    Q_PROPERTY(qreal                    scale           READ scale           WRITE setScale)
    Q_PROPERTY(qreal                    resolutionLimit READ resolutionLimit WRITE setResolutionLimit)
    Q_PROPERTY(bb::cascades::ImageView* helper          READ helper          WRITE setHelper)
    Q_PROPERTY(bool                     changed         READ changed)

    Q_ENUMS(Mode)

public:
    explicit DecolorizeEditor();
    virtual ~DecolorizeEditor();

    int  mode() const;
    void setMode(const int &mode);

    int  brushSize() const;
    void setBrushSize(const int &size);

    int  helperSize() const;
    void setHelperSize(const int &size);

    qreal brushOpacity() const;
    void  setBrushOpacity(const qreal &opacity);

    qreal scale() const;
    void  setScale(const qreal &scale);

    qreal resolutionLimit() const;
    void  setResolutionLimit(const qreal &limit);

    bb::cascades::ImageView *helper() const;
    void                     setHelper(bb::cascades::ImageView *helper);

    bool changed() const;

    Q_INVOKABLE void openImage(const QString &image_file);
    Q_INVOKABLE void saveImage(const QString &image_file);

    Q_INVOKABLE void changeImageAt(bool save_undo, int center_x, int center_y);

    Q_INVOKABLE void undo();

    Q_INVOKABLE void            addFragment(int x, int y, bb::cascades::ImageView *fragment);
    Q_INVOKABLE void            delFragment(int x, int y);
    Q_INVOKABLE QList<QObject*> getFragments();

    enum Mode {
        ModeScroll,
        ModeOriginal,
        ModeEffected
    };

public slots:
    void effectedImageReady(const QImage &effected_image);

signals:
    void imageOpened();
    void imageOpenFailed();

    void imageSaved();
    void imageSaveFailed();

    void undoAvailabilityChanged(bool available);

    void prepareFragments(int fragmentSize, int imageWidth, int imageHeight);

private:
    void SaveUndoImage();

    void RepaintImage(bool full, QRect rect = QRect());
    void RepaintHelper(int center_x, int center_y);

    static const int UNDO_DEPTH    = 4,
                     FRAGMENT_SIZE = 64;

    bool                                            IsChanged;
    int                                             CurrentMode, BrushSize, HelperSize;
    qreal                                           BrushOpacity, Scale, ResolutionLimit;
    QImage                                          LoadedImage, OriginalImage, EffectedImage, CurrentImage, BrushTemplateImage, BrushImage;
    QStack<QImage>                                  UndoStack;
    bb::cascades::ImageView                        *Helper;
    QMap<QPair<int, int>, bb::cascades::ImageView*> FragmentsMap;
};

class GrayscaleImageGenerator : public QObject
{
    Q_OBJECT

public:
    explicit GrayscaleImageGenerator(QObject *parent = 0);
    virtual ~GrayscaleImageGenerator();

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
