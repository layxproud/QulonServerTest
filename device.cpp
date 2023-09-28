#include "device.h"
#include <sstream>
#include <iomanip>

Device::Device(QObject *parent)
    : QObject{parent}
{
    connect(&_client, &TcpClient::connectionChanged, this, &Device::onConnectionChanged);
    connect(&_client, &TcpClient::dataSent, this, &Device::onDataSent);
    connect(&_client, &TcpClient::dataReceived, this, &Device::onDataReceived);
    connect(&_client, &TcpClient::errorOccurred, this, &Device::onError);
    connect(&_client, &TcpClient::noConnection, this, &Device::onNoConnection);
    connect(&_client, &TcpClient::wrongCRC, this, &Device::onWrongCRC);
    connect(&_client, &TcpClient::wrongTx, this, &Device::onWrongTx);
    connect(&_client, &TcpClient::unknownCommand, this, &Device::onUnknownCommand);
    connect(&_client, &TcpClient::replyError, this, &Device::onReplyError);
}


Device::Device(const QString &phone, const QString &name, Logger *logger, QObject *parent)
    : Device{parent}
{
    _name = name;
    _phone = phone;
    loggerInstance = logger;

    connectionTimer = new QTimer(this);
    disconnectionTimer = new QTimer(this);
    statusTimer = new QTimer(this);
    connect(connectionTimer, &QTimer::timeout, this, &Device::onConnectionTimerTimeout);
    connect(disconnectionTimer, &QTimer::timeout, this, &Device::onDisconnectionTimerTimeout);
    connect(statusTimer, &QTimer::timeout, this, &Device::onStatusTimerTimeout);

    _client.setPhone(_phone);
    _phoneId = phoneToId();
}

Device::~Device()
{
    if (_client.isConnected())
    {
        _client.disconnectFromServer();
    }
    if (connectionTimer->isActive())
    {
        connectionTimer->stop();
    }
    if (disconnectionTimer->isActive())
    {
        disconnectionTimer->stop();
    }
    if (statusTimer->isActive())
    {
        statusTimer->stop();
    }
    delete connectionTimer;
    delete disconnectionTimer;
    delete statusTimer;
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

void Device::setDisconnectionInterval(const int &from, const int &to)
{
    _disconnectionFromInterval = from;
    _disconnectionToInterval = to;
}

void Device::setSendStatusInterval(const int &interval)
{
    _sendStatusInterval = interval;
}

void Device::startConnectionTimer()
{
    QRandomGenerator randomGenerator(_phoneId);
    int randomInterval = randomGenerator.generate() % _connectionInterval;
    connectionTimer->start(randomInterval);
}

void Device::startDisconnectionTimer()
{
    QRandomGenerator randomGenerator(_phoneId);
    int randomInterval = randomGenerator.bounded(_disconnectionFromInterval, _disconnectionToInterval);
    disconnectionTimer->start(randomInterval);
}

void Device::startStatusTimer()
{
    statusTimer->start(_sendStatusInterval);
}


void Device::setLogger(Logger *logger)
{
    loggerInstance = logger;
}

int Device::phoneToId()
{
    int deviceId = 0;

    for (int i = 0; i < _phone.length(); ++i)
    {
        int asciiValue = _phone.at(i).toLatin1();
        deviceId = (deviceId << 8) | asciiValue;
    }
    return deviceId;
}


void Device::onConnectionChanged(const bool &status)
{
    if (status)
    {
        loggerInstance->logInfo(tr("Устройство с ID ") + _phone + tr(" подключено к серверу. Выполняется синхронизация..."));
        emit connectionChanged(status);
    }
    else
    {
        loggerInstance->logInfo(tr("Устройство с ID ") + _phone + tr(" отключилось от сервера."));
        emit connectionChanged(status);
    }
}


void Device::onDataReceived(const QByteArray &data)
{
    loggerInstance->logInfo(tr("ID ") + _phone + tr(" Получило сообщение: ") + loggerInstance->byteArrToStr(data));
}


void Device::onDataSent(const QByteArray &data)
{
    loggerInstance->logInfo(tr("ID ") + _phone + tr(" Отправило сообщение: ") + loggerInstance->byteArrToStr(data));
}


void Device::onError(const QString &errorString)
{
    loggerInstance->logError(tr("Ошибка сокета: ") + errorString);
}


void Device::onUnknownCommand(const UCHAR &command)
{
    QString commandString = QString("0x%1").arg(command, 2, 16, QChar('0'));
    loggerInstance->logWarning(tr("Встречена незнакомая команда: ") + commandString + tr(" Отправляю стандартный ответ..."));
}


void Device::onNoConnection()
{
    loggerInstance->logWarning(tr("Устройство c ID ") + _phone + tr(" не подключено к серверу!"));
}


void Device::onReplyError()
{
    loggerInstance->logError(tr("Сервер сообщил о возникшей ошибке"));
}


void Device::onWrongCRC(const UCHAR &expected1, const UCHAR &received1, const UCHAR &expected2, const UCHAR &received2)
{
    {
        QString message = tr("Неправильная контрольная сумма. Ожидалось: %1%2 | Получено: %3%4")
            .arg(QString::number(expected1, 16).rightJustified(2, '0'))
            .arg(QString::number(expected2, 16).rightJustified(2, '0'))
            .arg(QString::number(received1, 16).rightJustified(2, '0'))
            .arg(QString::number(received2, 16).rightJustified(2, '0'));

        loggerInstance->logError(message);
    }
}


void Device::onWrongTx(const UCHAR &expected, const UCHAR &received)
{
    QString message = tr("Tx Rx не совпадают. Ожидалось: %1 | Получено: %2")
        .arg(QString::number(expected, 16).rightJustified(2, '0'))
        .arg(QString::number(received, 16).rightJustified(2, '0'));

    loggerInstance->logError(message);
}


void Device::onConnectionTimerTimeout()
{
    _client.connectToServer(_ip, _port);
    connectionTimer->stop();
    startStatusTimer();
    startDisconnectionTimer();
}


void Device::onDisconnectionTimerTimeout()
{
    _client.disconnectFromServer();
    disconnectionTimer->stop();
    statusTimer->stop();
    startConnectionTimer();
}


void Device::onStatusTimerTimeout()
{
    _client.sendState(true);
}
