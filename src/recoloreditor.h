#ifndef RECOLOREDITOR_H
#define RECOLOREDITOR_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtGui/QMouseEvent>
#include <QtGui/QImage>

#include "editor.h"

class RecolorEditor : public Editor
{
    Q_OBJECT

    Q_PROPERTY(int hue READ hue WRITE setHue)

    Q_ENUMS(Mode)

public:
    explicit RecolorEditor(QQuickItem *parent = nullptr);
    ~RecolorEditor() override = default;

    int hue() const;
    void setHue(int hue);

    enum Mode {
        ModeScroll,
        ModeOriginal,
        ModeEffected
    };

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void processOpenedImage() override;

private:
    void ChangeImageAt(bool save_undo, int center_x, int center_y);

    quint16 PackRGB16(quint8 r, quint8 g, quint8 b);
    std::tuple<quint8, quint8, quint8> UnpackRGB16(quint16 rgb);

    quint32 PackHSV(qint16 h, quint8 s, quint8 v);
    std::tuple<qint16, quint8, quint8> UnpackHSV(quint32 hsv);

    QRgb AdjustHue(QRgb rgb);

    int                     CurrentHue;
    QImage                  OriginalImage;
    QHash<quint16, quint32> RGB16ToHSVMap;
};

#endif // RECOLOREDITOR_H
