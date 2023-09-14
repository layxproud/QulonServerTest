#include "device.h"

Device::Device(QObject *parent)
    : QObject{parent}
{
}

Device::Device(const QString &phone, const QString &name, QObject *parent)
    : Device{parent}
{
    _phone = phone;
    _name = name;
}

QString Device::getPhone() const
{
    return _phone;
}

QString Device::getName() const
{
    return _name;
}

bool Device::isConnected() const
{
    return _connected;
}
