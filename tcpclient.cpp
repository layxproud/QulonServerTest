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

    // Reset Tx Rx
    _currTx = 0x00;
    _currRx = 0x80;
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

void TcpClient::parseMessage(const QByteArray &message)
{
    FL_MODBUS_MESSAGE modbusMessage;
    QByteArray choppedMessage = message.mid(1, message.size() - 2); // remove the first and last byte (0xC0)
    QByteArray syncMessage = QByteArray::fromHex("00800010"); // the sync message is always these bytes
    qDebug() << choppedMessage;

    // Sync message case
    if (choppedMessage == syncMessage)
    {
        sendSyncCommand();
    }
    // FL_MODBUS_MESSAGE case
    else if (static_cast<unsigned char>(choppedMessage[3]) == 0x6E)
    {
        memcpy(&modbusMessage, choppedMessage.constData(), sizeof(FL_MODBUS_MESSAGE));
        int dataLength = modbusMessage.len;
        QByteArray data = choppedMessage.mid(sizeof(FL_MODBUS_MESSAGE), dataLength);
        sendIdentificationMessage();

        _currTx = modbusMessage.tx_id;
        _currRx = modbusMessage.rx_id;
    }
    // FL_MODBUS_MESSAGE_SHORT case
    // TODO. I don't get the difference between MODBUS_MESSAGE and FL_MODBUS_MESSAGE_SHORT
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
