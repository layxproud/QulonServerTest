#include "tcpclient.h"

TcpClient::TcpClient(Logger* logger, const QString& phone, QObject *parent)
    : QObject{parent},
    devicePhone{phone},
    connectionStatus{false},
    logAllowed(true),
    logger{logger}
{
    connect(&tcpSocket, &QTcpSocket::connected, this, &TcpClient::onSocketConnected);
    connect(&tcpSocket, &QTcpSocket::disconnected, this, &TcpClient::onSocketDisconnected);
    connect(&tcpSocket, &QAbstractSocket::errorOccurred, this, &TcpClient::onSocketError);
    connect(&tcpSocket, &QTcpSocket::readyRead, this, &TcpClient::onSocketReadyRead);
}


TcpClient::~TcpClient()
{
    if (tcpSocket.state() == QAbstractSocket::ConnectedState)
        disconnectFromServer();
}

bool TcpClient::isConnected() const
{
    return connectionStatus;
}

void TcpClient::connectToServer(const QString &serverAddress, quint16 serverPort)
{
    tcpSocket.connectToHost(serverAddress, serverPort);
}

void TcpClient::disconnectFromServer()
{
    tcpSocket.disconnectFromHost();
}

void TcpClient::editLogStatus(const bool &status)
{
    logAllowed = status;
}

bool TcpClient::checkConnection()
{
    if (!connectionStatus)
    {
        if (logAllowed)
            logger->logWarning(tr("Устройство c ID ") + devicePhone + tr(" не подключено к серверу!"));
        return false;
    }
    else return true;
}

void TcpClient::onSocketConnected()
{
    connectionStatus = true;
    if (logAllowed)
        logger->logInfo(tr("Устройство с ID ") + devicePhone + tr(" подключено к серверу. Выполняется синхронизация..."));
    emit connectionChanged(connectionStatus);
}

void TcpClient::onSocketDisconnected()
{
    connectionStatus = false;
    if (logAllowed)
        logger->logInfo(tr("Устройство с ID ") + devicePhone + tr(" отключилось от сервера."));
    emit connectionChanged(connectionStatus);
}

void TcpClient::onSocketReadyRead()
{
    receivedMessage = tcpSocket.readAll();
    if (logAllowed)
        logger->logInfo(tr("ID ") + devicePhone + tr(" Получило сообщение: ") + logger->byteArrToStr(receivedMessage));
    emit messageReceived(receivedMessage);
}

void TcpClient::onSocketError()
{
    QString errorString = tcpSocket.errorString();
    if (logAllowed)
        logger->logError(tr("Ошибка сокета: ") + errorString);
}

void TcpClient::onWrongCRC(const UCHAR &expected1, const UCHAR &received1, const UCHAR &expected2, const UCHAR &received2)
{
    QString message = tr("Неправильная контрольная сумма. Ожидалось: %1%2 | Получено: %3%4").arg(
        QString::number(expected1, 16).rightJustified(2, '0'),
        QString::number(expected2, 16).rightJustified(2, '0'),
        QString::number(received1, 16).rightJustified(2, '0'),
        QString::number(received2, 16).rightJustified(2, '0'));

    if (logAllowed)
        logger->logError(message);
}

void TcpClient::onWrongTx(const UCHAR &expected, const UCHAR &received)
{
    QString message = tr("Tx не совпадают. Ожидалось: %1 | Получено: %2").arg(
        QString::number(expected, 16).rightJustified(2, '0'),
        QString::number(received, 16).rightJustified(2, '0'));

    if (logAllowed)
        logger->logError(message);
}

void TcpClient::onUnknownCommand(const UCHAR &command)
{
    QString commandString = QString("0x%1").arg(command, 2, 16, QChar('0'));
    if (logAllowed)
        logger->logWarning(tr("Устройство с ID ") + devicePhone + tr(" встретило незнакомую команду: ") + commandString + tr(" Отправляю стандартный ответ..."));
}

void TcpClient::sendMessage(const QByteArray &message)
{
    if (!checkConnection())
        return;
    currentMessage = message;
    tcpSocket.write(message);

    if (logAllowed)
        logger->logInfo(tr("ID ") + devicePhone + tr(" Отправило сообщение: ") + logger->byteArrToStr(currentMessage));
}
