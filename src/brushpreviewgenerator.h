#ifndef BRUSHPREVIEWGENERATOR_H
#define BRUSHPREVIEWGENERATOR_H

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtQuick/QQuickPaintedItem>

class BrushPreviewGenerator : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(int   size     READ size     WRITE setSize)
    Q_PROPERTY(int   maxSize  READ maxSize  WRITE setMaxSize)
    Q_PROPERTY(qreal hardness READ hardness WRITE setHardness)

public:
    explicit BrushPreviewGenerator(QQuickItem *parent = nullptr);

    BrushPreviewGenerator(const BrushPreviewGenerator&) = delete;
    BrushPreviewGenerator(const BrushPreviewGenerator&&) noexcept = delete;

    BrushPreviewGenerator& operator=(const BrushPreviewGenerator&) = delete;
    BrushPreviewGenerator& operator=(const BrushPreviewGenerator&&) noexcept = delete;

    ~BrushPreviewGenerator() noexcept override = default;

    int size() const;
    void setSize(int size);

    int maxSize() const;
    void setMaxSize(int max_size);

    qreal hardness() const;
    void setHardness(qreal hardness);

    void paint(QPainter *painter) override;

private:
    int   Size, MaxSize;
    qreal Hardness;
};

#endif // BRUSHPREVIEWGENERATOR_H
