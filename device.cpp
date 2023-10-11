#include "device.h"
#include <sstream>
#include <iomanip>

Device::Device(QObject *parent)
    : QObject{parent},
    connectionTimer(new QTimer(this)),
    disconnectionTimer(new QTimer(this)),
    statusTimer(new QTimer(this))
{

}


Device::Device(const QString &phone, const QString &name, Logger *logger, QObject *parent)
    : Device{parent}
{
    _name = name;
    _phone = phone;
    _logger = logger;
    _phoneId = phoneToId();
    _client = new TcpClient(_logger, _phone, this);
    connect(_client, &TcpClient::connectionChanged, this, &Device::onConnectionChanged);
    _connected = _client->isConnected();

    connect(connectionTimer, &QTimer::timeout, this, &Device::onConnectionTimerTimeout);
    connect(disconnectionTimer, &QTimer::timeout, this, &Device::onDisconnectionTimerTimeout);
    connect(statusTimer, &QTimer::timeout, this, &Device::onStatusTimerTimeout);
}

Device::~Device()
{
    stopWork();
    delete connectionTimer;
    delete disconnectionTimer;
    delete statusTimer;

    if (_client->isConnected())
        _client->disconnectFromServer();
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

void Device::startWork()
{
    startConnectionTimer();
}

void Device::stopWork()
{
    if (connectionTimer->isActive())
    {
        connectionTimer->stop();
    }
    if (disconnectionTimer->isActive())
    {
        disconnectionTimer->stop();
        _client->disconnectFromServer();
    }
    if (statusTimer->isActive())
    {
        statusTimer->stop();
    }
}

void Device::debugConnect(const QString &serverAddress, quint16 serverPort)
{
    _client->connectToServer(serverAddress, serverPort);
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
    emit connectionChanged(status);
    _connected = status;
}

void Device::onConnectionTimerTimeout()
{
    _client->connectToServer(_ip, _port);
    connectionTimer->stop();
    startStatusTimer();
    startDisconnectionTimer();
}

void Device::onDisconnectionTimerTimeout()
{
    _client->disconnectFromServer();
    disconnectionTimer->stop();
    statusTimer->stop();
    startConnectionTimer();
}

void Device::onStatusTimerTimeout()
{
    _client->sendState(true);
}
