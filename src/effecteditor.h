#ifndef EFFECTEDITOR_H
#define EFFECTEDITOR_H

#include <QtCore/QObject>
#include <QtGui/QMouseEvent>
#include <QtGui/QImage>

#include "editor.h"

class EffectEditor : public Editor
{
    Q_OBJECT

    Q_ENUMS(Mode)

public:
    explicit EffectEditor(QQuickItem *parent = nullptr);
    ~EffectEditor() override = default;

    enum Mode {
        ModeScroll,
        ModeOriginal,
        ModeEffected
    };

protected slots:
    void effectedImageReady(const QImage &effected_image);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void ChangeImageAt(bool save_undo, int center_x, int center_y);

    QImage OriginalImage, EffectedImage;
};

#endif // EFFECTEDITOR_H
