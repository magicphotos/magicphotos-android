#include "invocationhelper.h"

InvocationHelper::InvocationHelper(QObject *parent) : QObject(parent)
{
    InvocationManager = new bb::system::InvokeManager();

    QObject::connect(InvocationManager, SIGNAL(invoked(const bb::system::InvokeRequest &)), this, SLOT(invoked(const bb::system::InvokeRequest &)));
}

InvocationHelper::~InvocationHelper()
{
    delete InvocationManager;
}

QString InvocationHelper::imageFile() const
{
    return ImageFile;
}

void InvocationHelper::setImageFile(const QString &image_file)
{
    if (ImageFile != image_file) {
        ImageFile = image_file;

        emit imageFileChanged();
    }
}

void InvocationHelper::invoked(const bb::system::InvokeRequest &request)
{
    ImageFile = request.uri().toLocalFile();

    emit imageFileChanged();
}
