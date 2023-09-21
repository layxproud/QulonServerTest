#include "device.h"

Device::Device(QObject *parent)
    : QObject{parent}
{
    connect(&_client, &TcpClient::connected, this, &Device::onConnected);
    connect(&_client, &TcpClient::disconnected, this, &Device::onDisconnected);
    connect(&_client, &TcpClient::dataSent, this, &Device::onDataSent);
    connect(&_client, &TcpClient::dataReceived, this, &Device::onDataReceived);
    connect(&_client, &TcpClient::errorOccurred, this, &Device::onError);
}


Device::Device(const QString &phone, const QString &name, Logger *logger, QObject *parent)
    : Device{parent}
{
    _phone = phone;
    _name = name;
    setLogger(logger);
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


void Device::setLogger(Logger *logger)
{
    loggerInstance = logger;
}


void Device::connectToServer(const QString &serverAddress, quint16 serverPort)
{
    _client.connectToServer(serverAddress, serverPort);
}


void Device::disconnectFromServer()
{
    _client.disconnectFromServer();
}


void Device::sendSyncCommand()
{
    if (!_connected)
    {
        loggerInstance->logWarning(tr("Устройство не подключено к серверу!"));
        return;
    }

    _client.sendSyncCommand();
}


void Device::sendIdentificationMessage()
{
    if (!_connected)
    {
        loggerInstance->logWarning(tr("Устройство не подключено к серверу!"));
        return;
    }

    _client.sendIdentificationMessage(_phone);
}

void Device::onConnected()
{
    _connected = true;
    loggerInstance->logInfo(tr("Устройство с ID ") + _phone + tr(" подключено к серверу. Выполняется синхронизация..."));
    emit connected();

    sendSyncCommand();
}

void Device::onDisconnected()
{
    _connected = false;
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
