#include "device.h"
#include <sstream>
#include <iomanip>

Device::Device(QObject *parent)
    : QObject{parent},
    _autoRegen(true),
    connectionTimer(new QTimer(this)),
    disconnectionTimer(new QTimer(this)),
    sendStatusTimer(new QTimer(this)),
    changeStatusTimer(new QTimer(this))
{

}

Device::Device(const QString &phone, const QString &name, Logger *logger, QObject *parent)
    : Device{parent}
{
    _name = name;
    _phone = phone;
    _logger = logger;
    _client = new TcpClient(_logger, _phone, this);
    connect(_client, &TcpClient::connectionChanged, this, &Device::onConnectionChanged);
    _connected = _client->isConnected();

    connect(connectionTimer, &QTimer::timeout, this, &Device::onConnectionTimerTimeout);
    connect(disconnectionTimer, &QTimer::timeout, this, &Device::onDisconnectionTimerTimeout);
    connect(sendStatusTimer, &QTimer::timeout, this, &Device::onSendStatusTimerTimeout);
    connect(changeStatusTimer, &QTimer::timeout, this, &Device::onChangeStatusTimeTimeout);
}

Device::~Device()
{
    stopWork();
    delete connectionTimer;
    delete disconnectionTimer;
    delete sendStatusTimer;
    delete changeStatusTimer;

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

void Device::setAutoRegen(const bool &regen)
{
    _autoRegen = regen;
}

void Device::setDefaults(const DeviceDefaults &defaults)
{
    setConnectionInterval(defaults.connectionInterval);
    setDisconnectionInterval(defaults.disconnectionFromInterval, defaults.disconnectionToInterval);
    setSendStatusInterval(defaults.sendStatusInterval);
    setChangeStatusInterval(defaults.changeStatusInterval);
    setAutoRegen(defaults.autoRegen);
    editLogStatus(defaults.logStatus);
}

void Device::setLampsList()
{
    _lampList.init(10, 50);
    QByteArray file = _lampList.getFile();
    _client->addFileToMap("STATE2.DAT", file);
    // DEBUG FILES
    _client->addFileToMap("INFO.DAT", file);
    _client->addFileToMap("STATE1.DAT", file);
    _client->addFileToMap("STATE2.NET", file);
    _client->addFileToMap("SOBAKA.DAT", file);
    _client->addFileToMap("A.DAT", file);
    _client->addFileToMap("B.PDF", file);
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
    if (sendStatusTimer->isActive())
    {
        sendStatusTimer->stop();
    }
    if (changeStatusTimer->isActive())
    {
        changeStatusTimer->stop();
    }
}

void Device::debugConnect(const QString &serverAddress, quint16 serverPort)
{
    _client->connectToServer(serverAddress, serverPort);
}

void Device::editByte(const UCHAR &stateByte, const QByteArray &byte)
{
    _client->editByte(stateByte, byte);
}

void Device::editLogStatus(const bool &status)
{
    _client->editLogStatus(status);
}

void Device::setConnectionInterval(const int &interval)
{
    _defaults.connectionInterval = interval;
}

void Device::setDisconnectionInterval(const int &from, const int &to)
{
    _defaults.disconnectionFromInterval = from;
    _defaults.disconnectionToInterval = to;
}

void Device::setSendStatusInterval(const int &interval)
{
    _defaults.sendStatusInterval = interval;
}

void Device::setChangeStatusInterval(const int &interval)
{
    _defaults.changeStatusInterval = interval;
}

void Device::startConnectionTimer()
{
    int randomInterval = QRandomGenerator::global()->bounded(_defaults.connectionInterval);
    connectionTimer->start(randomInterval);
}

void Device::startDisconnectionTimer()
{
    int randomInterval = QRandomGenerator::global()->bounded(_defaults.disconnectionFromInterval, _defaults.disconnectionToInterval);
    disconnectionTimer->start(randomInterval);
}

void Device::startSendStatusTimer()
{
    sendStatusTimer->start(_defaults.sendStatusInterval);
}

void Device::startChangeStatusTimer()
{
    changeStatusTimer->start(_defaults.changeStatusInterval);
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
    startSendStatusTimer();
    startDisconnectionTimer();
    startChangeStatusTimer();
}

void Device::onDisconnectionTimerTimeout()
{
    _client->disconnectFromServer();
    disconnectionTimer->stop();
    sendStatusTimer->stop();
    changeStatusTimer->stop();
    startConnectionTimer();
}

void Device::onSendStatusTimerTimeout()
{
    _client->sendState(true);
}

void Device::onChangeStatusTimeTimeout()
{
    if (_autoRegen)
        _client->randomiseState();
    else return;
}
