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
    connect(&_client, &TcpClient::wrongCRC, this, &Device::onWrongCRC);
    connect(&_client, &TcpClient::wrongRx, this, &Device::onWrongRx);
}


Device::Device(const QString &phone, const QString &name, Logger *logger, QObject *parent)
    : Device{parent}
{
    _name = name;
    _phone = phone;
    loggerInstance = logger;

    connectionTimer = new QTimer(this);
    disconnectionTimer = new QTimer(this);
    connect(connectionTimer, &QTimer::timeout, this, &Device::onConnectionTimerTimeout);
    connect(disconnectionTimer, &QTimer::timeout, this, &Device::onDisconnectionTimerTimeout);

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

void Device::setIp(const QString &ip)
{
    _ip = ip;
}

void Device::setPort(const quint16 &port)
{
    _port = port;
}

void Device::setConnectionInterval(const int &interval)
{
    _connectionInterval = interval;
}

void Device::setDisconnectionInterval(const int &interval)
{
    _disconnectionInterval = interval;
}

void Device::startConnectionTimer()
{
    int randomInterval = qrand() % ((_connectionInterval * 60 * 1000) + 1);
    connectionTimer->start(randomInterval);
}

void Device::startDisconnectionTimer()
{
    int randomInterval = qrand() % ((_disconnectionInterval * 60 * 1000) + 1);
    disconnectionTimer->start(randomInterval);
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

void Device::onWrongCRC(const UCHAR &expected1, const UCHAR &received1, const UCHAR &expected2, const UCHAR &received2)
{
    loggerInstance->logError(tr("Неправильная контрольная сумма. Ожидалось: ") + expected1 + expected2
                             + tr(" | Получено: ") + received1 + received2);
}

void Device::onWrongRx(const UCHAR &expected, const UCHAR &received)
{
    loggerInstance->logWarning(tr("Rx не совпадает. Ожидалось: ") + expected
                               + tr(" | Получено: ") + received);
}

void Device::onConnectionTimerTimeout()
{
    _client.connectToServer(_ip, _port);
    connectionTimer->stop();
    startDisconnectionTimer();
}

void Device::onDisconnectionTimerTimeout()
{
    _client.disconnectFromServer();
    disconnectionTimer->stop();
    startConnectionTimer();
}
