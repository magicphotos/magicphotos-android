#ifndef CUSTOMTIMER_H
#define CUSTOMTIMER_H

#include <QtCore/QObject>
#include <QtCore/QTimer>

#include <bb/cascades/CustomControl>

class CustomTimer : public bb::cascades::CustomControl
{
    Q_OBJECT

    Q_PROPERTY(bool active     READ active                         NOTIFY activeChanged)
    Q_PROPERTY(bool singleShot READ singleShot WRITE setSingleShot NOTIFY singleShotChanged)
    Q_PROPERTY(int  interval   READ interval   WRITE setInterval   NOTIFY intervalChanged)

public:
    explicit CustomTimer(QObject *parent = 0);
    virtual ~CustomTimer();

    bool active() const;

    bool singleShot() const;
    void setSingleShot(const bool &single_shot);

    int  interval() const;
    void setInterval(const int &interval);

public slots:
    void start();
    void stop();

signals:
    void timeout();
    void activeChanged();
    void singleShotChanged();
    void intervalChanged();

private:
    QTimer Timer;
};

#endif // CUSTOMTIMER_H
