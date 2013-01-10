#ifndef MAGICPHOTOS_HPP
#define MAGICPHOTOS_HPP

#include <QtCore/QObject>

namespace bb { namespace cascades { class Application; }}

class MagicPhotos : public QObject
{
    Q_OBJECT
public:
    MagicPhotos(bb::cascades::Application *app);
    virtual ~MagicPhotos() {}
};

#endif // MAGICPHOTOS_HPP
