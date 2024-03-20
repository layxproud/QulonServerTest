#include "device.h"
#include <sstream>
#include <iomanip>

Device::Device(QObject *parent)
    : QObject{parent},
    autoRegen(true),
    connectionTimer(new QTimer(this)),
    disconnectionTimer(new QTimer(this)),
    sendStatusTimer(new QTimer(this)),
    changeStatusTimer(new QTimer(this)),
    lampList(new LampList(this))
{

}

Device::Device(const QString &phone, const QString &name, Logger *logger, QObject *parent)
    : Device{parent}
{
    deviceName = name;
    devicePhone = phone;
    this->logger = logger;

    tcpClient = new TcpClient(logger, devicePhone, this);
    connectionStatus = tcpClient->isConnected();

    modbusHandler = new ModbusHandler(this);
    modbusHandler->initModbusHandler(devicePhone);

    connect(tcpClient, &TcpClient::connectionChanged, this, &Device::onConnectionChanged);
    connect(tcpClient, &TcpClient::messageReceived, modbusHandler, &ModbusHandler::parseMessage);
    connect(modbusHandler, &ModbusHandler::messageToSend, tcpClient, &TcpClient::sendMessage);
    connect(modbusHandler, &ModbusHandler::wrongCRC, tcpClient, &TcpClient::onWrongCRC);
    connect(modbusHandler, &ModbusHandler::wrongTx, tcpClient, &TcpClient::onWrongTx);
    connect(modbusHandler, &ModbusHandler::unknownCommand, tcpClient, &TcpClient::onUnknownCommand);

    connect(connectionTimer, &QTimer::timeout, this, &Device::onConnectionTimerTimeout);
    connect(disconnectionTimer, &QTimer::timeout, this, &Device::onDisconnectionTimerTimeout);
    connect(sendStatusTimer, &QTimer::timeout, this, &Device::onSendStatusTimerTimeout);
    connect(changeStatusTimer, &QTimer::timeout, this, &Device::onChangeStatusTimeTimeout);
    connect(lampList, &LampList::nodesUpdated, this, &Device::onNodesUpdated);
}

Device::~Device()
{
    isBeingDestroyed = true;
    stopWork();
    delete connectionTimer;
    delete disconnectionTimer;
    delete sendStatusTimer;
    delete changeStatusTimer;

    if (tcpClient->isConnected())
        tcpClient->disconnectFromServer();
}

QString Device::getPhone() const
{
    return devicePhone;
}

QString Device::getName() const
{
    return deviceName;
}

LampList* Device::getLampList() const
{
    return lampList;
}

bool Device::isConnected() const
{
    return connectionStatus;
}

void Device::setIp(const QString &ip)
{
    serverIp = ip;
}

void Device::setPort(const quint16 &port)
{
    serverPort = port;
}

void Device::setAutoRegen(const bool &regen)
{
    autoRegen = regen;
}

void Device::setDefaults(const DeviceDefaults &defaults)
{
    setConnectionInterval(defaults.connectionInterval);
    setDisconnectionInterval(defaults.disconnectionFromInterval, defaults.disconnectionToInterval);
    setSendStatusInterval(defaults.sendStatusInterval);
    setChangeStatusInterval(defaults.changeStatusInterval);
    setAutoRegen(defaults.autoRegen);
}

void Device::setLampsList(int size, int level, UCHAR status)
{
    lampList->init(size, level, status);
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
        tcpClient->disconnectFromServer();
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
    tcpClient->connectToServer(serverAddress, serverPort);
}

void Device::editLogStatus(const bool &status)
{
    tcpClient->editLogStatus(status);
}

void Device::editState(const UCHAR &stateByte, const QByteArray &data)
{
    modbusHandler->editState(stateByte, data);
}

void Device::sendState()
{
    modbusHandler->formStateMessage(true);
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
    if (isBeingDestroyed)
        return;
    if (status == false)
        stopWork();
    emit connectionChanged(status);
    connectionStatus = status;
}

void Device::onConnectionTimerTimeout()
{
    tcpClient->connectToServer(serverIp, serverPort);
    connectionTimer->stop();
    startSendStatusTimer();
    startDisconnectionTimer();
    startChangeStatusTimer();
}

void Device::onDisconnectionTimerTimeout()
{
    tcpClient->disconnectFromServer();
    disconnectionTimer->stop();
    sendStatusTimer->stop();
    changeStatusTimer->stop();
    startConnectionTimer();
}

void Device::onSendStatusTimerTimeout()
{
    modbusHandler->formStateMessage(true);
}

void Device::onChangeStatusTimeTimeout()
{
    if (autoRegen)
        modbusHandler->randomiseRelayStates();
    else return;
}

void Device::onNodesUpdated()
{
    QByteArray file = lampList->getFile();
    modbusHandler->addFileToMap("STATE2.DAT", file);
}
