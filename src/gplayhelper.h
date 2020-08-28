#ifndef GPLAYHELPER_H
#define GPLAYHELPER_H

#include <QtCore/QObject>

class GPlayHelper : public QObject
{
    Q_OBJECT

private:
    explicit GPlayHelper(QObject *parent = nullptr);
    ~GPlayHelper() noexcept override = default;

public:
    GPlayHelper(const GPlayHelper &) = delete;
    GPlayHelper(GPlayHelper &&) noexcept = delete;

    GPlayHelper &operator=(const GPlayHelper &) = delete;
    GPlayHelper &operator=(GPlayHelper &&) noexcept = delete;

    static GPlayHelper &GetInstance();

    Q_INVOKABLE void requestReview() const;
};

#endif // GPLAYHELPER_H
