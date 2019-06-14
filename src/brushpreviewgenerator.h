#ifndef BRUSHPREVIEWGENERATOR_H
#define BRUSHPREVIEWGENERATOR_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtQuick/QQuickPaintedItem>

class BrushPreviewGenerator : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(int   brushSize     READ brushSize     WRITE setBrushSize)
    Q_PROPERTY(int   maxBrushSize  READ maxBrushSize  WRITE setMaxBrushSize)
    Q_PROPERTY(qreal brushHardness READ brushHardness WRITE setBrushHardness)

public:
    explicit BrushPreviewGenerator(QQuickItem *parent = nullptr);

    BrushPreviewGenerator(const BrushPreviewGenerator &) = delete;
    BrushPreviewGenerator(BrushPreviewGenerator &&) noexcept = delete;

    BrushPreviewGenerator &operator=(const BrushPreviewGenerator &) = delete;
    BrushPreviewGenerator &operator=(BrushPreviewGenerator &&) noexcept = delete;

    ~BrushPreviewGenerator() noexcept override = default;

    int brushSize() const;
    void setBrushSize(int size);

    int maxBrushSize() const;
    void setMaxBrushSize(int max_size);

    qreal brushHardness() const;
    void setBrushHardness(qreal hardness);

    void paint(QPainter *painter) override;

private:
    int   BrushSize, MaxBrushSize;
    qreal BrushHardness;
};

#endif // BRUSHPREVIEWGENERATOR_H
