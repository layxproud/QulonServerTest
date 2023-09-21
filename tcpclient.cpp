#include "tcpclient.h"

TcpClient::TcpClient(QObject *parent)
    : QObject{parent}
{
    connect(&_socket, &QTcpSocket::connected, this, &TcpClient::onSocketConnected);
    connect(&_socket, &QTcpSocket::disconnected, this, &TcpClient::onSocketDisconnected);
    connect(&_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
    connect(&_socket, &QTcpSocket::readyRead, this, &TcpClient::onSocketReadyRead);
}

void TcpClient::connectToServer(const QString &serverAddress, quint16 serverPort)
{
    _socket.connectToHost(serverAddress, serverPort);
}

void TcpClient::disconnectFromServer()
{
    _socket.disconnectFromHost();
}

void TcpClient::sendSyncCommand()
{
    std::vector<UCHAR> syncData;
    syncData.push_back(0x00);
    syncData.push_back(0x80);
    CalculateCRC(syncData);
    QByteArray byteArray;
    byteArray.append(static_cast<char>(0xC0));
    byteArray.append(reinterpret_cast<const char*>(syncData.data()), static_cast<int>(syncData.size()));
    byteArray.append(static_cast<char>(0xC0));

    _currentMessage = byteArray;
    _socket.write(byteArray);

    emit dataSent(_currentMessage);
}

void TcpClient::sendIdentificationMessage(const QString &phone)
{
    FL_MODBUS_PROT_ID_CMD_MESSAGE idMessage;
    memset(&idMessage, 0, sizeof(idMessage));
    strcpy_s(idMessage.phone, phone.toUtf8().constData());

    QByteArray byteArray;
    byteArray.append(reinterpret_cast<char*>(&idMessage), sizeof(idMessage));

    _currentMessage = byteArray;
    _socket.write(byteArray);

    emit dataSent(_currentMessage);
}

void TcpClient::onSocketConnected()
{
    emit connected();
}

void TcpClient::onSocketDisconnected()
{
    emit disconnected();
}

void TcpClient::onSocketReadyRead()
{
    _receivedMessage = _socket.readAll();

    emit dataReceived(_receivedMessage);
}

void TcpClient::onSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QString errorString = _socket.errorString();

    emit errorOccurred(errorString);
}
