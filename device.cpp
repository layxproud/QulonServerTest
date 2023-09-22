#include "device.h"

Device::Device(QObject *parent)
    : QObject{parent}
{
    connect(&_client, &TcpClient::connected, this, &Device::onConnected);
    connect(&_client, &TcpClient::disconnected, this, &Device::onDisconnected);
    connect(&_client, &TcpClient::dataSent, this, &Device::onDataSent);
    connect(&_client, &TcpClient::dataReceived, this, &Device::onDataReceived);
    connect(&_client, &TcpClient::errorOccurred, this, &Device::onError);
    connect(&_client, &TcpClient::noConnection, this, &Device::onNoConnection);
}


Device::Device(const QString &phone, const QString &name, Logger *logger, QObject *parent)
    : Device{parent}
{
    _phone = phone;
    _name = name;
    setLogger(logger);
    _client.setPhone(_phone);
}


QString Device::getPhone() const
{
    return _phone;
}


QString Device::getName() const
{
    return _name;
}


void Device::setLogger(Logger *logger)
{
    loggerInstance = logger;
}


void Device::onConnected()
{
    loggerInstance->logInfo(tr("Устройство с ID ") + _phone + tr(" подключено к серверу. Выполняется синхронизация..."));
    emit connected();
}


void Device::onDisconnected()
{
    loggerInstance->logInfo(tr("Устройство с ID ") + _phone + tr(" отключилось от сервера."));
    emit disconnected();
}


void Device::onDataReceived(const QByteArray &data)
{
    loggerInstance->logInfo(tr("Получено сообщение от сервера: ") + loggerInstance->byteArrToStr(data));
}


void Device::onDataSent(const QByteArray &data)
{
    loggerInstance->logInfo(tr("Отправлено сообщение: ") + loggerInstance->byteArrToStr(data));
}


void Device::onError(const QString &errorString)
{
    loggerInstance->logError(tr("Ошибка сокета: ") + errorString);
}


void Device::onNoConnection()
{
    loggerInstance->logWarning(tr("Устройство c ID ") + _phone + tr(" не подключено к серверу!"));
}
