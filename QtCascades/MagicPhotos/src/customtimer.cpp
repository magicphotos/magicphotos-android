#include "customtimer.h"

CustomTimer::CustomTimer(QObject *parent) : bb::cascades::CustomControl()
{
    Q_UNUSED(parent);

    QObject::connect(&Timer, SIGNAL(timeout()), this, SIGNAL(timeout()));
}

CustomTimer::~CustomTimer()
{
}

bool CustomTimer::active() const
{
    return Timer.isActive();
}

bool CustomTimer::singleShot() const
{
    return Timer.isSingleShot();
}

void CustomTimer::setSingleShot(const bool &single_shot)
{
    if (Timer.isSingleShot() != single_shot) {
        Timer.setSingleShot(single_shot);

        emit singleShotChanged();
    }
}

int CustomTimer::interval() const
{
    return Timer.interval();
}

void CustomTimer::setInterval(const int &interval)
{
    if (Timer.interval() != interval) {
        Timer.setInterval(interval);

        emit intervalChanged();
    }
}

void CustomTimer::start()
{
    if (!Timer.isActive()) {
        Timer.start();

        emit activeChanged();
    }
}

void CustomTimer::stop()
{
    if (Timer.isActive()) {
        Timer.stop();

        emit activeChanged();
    }
}
