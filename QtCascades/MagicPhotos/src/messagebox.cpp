#include "messagebox.h"

MessageBox::MessageBox(QObject *parent) : QObject(parent)
{
}

MessageBox::~MessageBox()
{
}

void MessageBox::showToast(const QString &body)
{
    Toast.setPosition(bb::system::SystemUiPosition::MiddleCenter);
    Toast.setBody(body);

    Toast.show();
}
