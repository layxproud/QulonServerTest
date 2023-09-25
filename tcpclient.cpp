#include "tcpclient.h"

TcpClient::TcpClient(QObject *parent)
    : QObject{parent}
{
    connect(&_socket, &QTcpSocket::connected, this, &TcpClient::onSocketConnected);
    connect(&_socket, &QTcpSocket::disconnected, this, &TcpClient::onSocketDisconnected);
    connect(&_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
    connect(&_socket, &QTcpSocket::readyRead, this, &TcpClient::onSocketReadyRead);
}


TcpClient::~TcpClient()
{
    if (_socket.state() == QAbstractSocket::ConnectedState)
        disconnectFromServer();
}


void TcpClient::setPhone(const QString &phone)
{
    _phone = phone;
}


bool TcpClient::isConnected() const
{
    return _connected;
}


void TcpClient::connectToServer(const QString &serverAddress, quint16 serverPort)
{
    _socket.connectToHost(serverAddress, serverPort);
}


void TcpClient::disconnectFromServer()
{
    _socket.disconnectFromHost();
}


void TcpClient::parseMessage(const QByteArray &message)
{
    QByteArray syncMessage = QByteArray::fromHex("00800010"); // the sync message is always these bytes
    QByteArray choppedMessage = message.mid(1, message.size() - 2); // remove the first and last byte (0xC0)

    // Sync message case
    if (choppedMessage == syncMessage)
    {
        sendSyncCommand();
    }

    // FL_MODBUS_MESSAGE case
    else if (static_cast<unsigned char>(choppedMessage[3]) == 0x6E)
    {
        // HEADER
        FL_MODBUS_MESSAGE modbusMessage;
        memcpy(&modbusMessage, choppedMessage.constData(), sizeof(FL_MODBUS_MESSAGE));

        // DATA
        int dataLength = modbusMessage.len;
        QByteArray rawData = choppedMessage.mid(sizeof(FL_MODBUS_MESSAGE), dataLength);
        QByteArray data = transformData(rawData);

        // CRC
        UCHAR crc[2];
        CalculateCRC(modbusMessage, data, crc);
        if ((crc[1] != choppedMessage.at(choppedMessage.size() - 1)) &&
            (crc[0] != choppedMessage.at(choppedMessage.size() - 2)))
        {
            emit wrongCRC(choppedMessage.at(choppedMessage.size() - 1), crc[1],
                    choppedMessage.at(choppedMessage.size() - 2), crc[0]);
        }

        // Check Rx
        if (_currRx != modbusMessage.rx_id)
        {
            emit wrongRx(_currRx, modbusMessage.rx_id);
        }

        performCommand(choppedMessage, data);
    }
    // FL_MODBUS_MESSAGE_SHORT case
    // TODO. I don't get the difference between MODBUS_MESSAGE and FL_MODBUS_MESSAGE_SHORT
}


void TcpClient::performCommand(const QByteArray &message, const QByteArray &data)
{
    switch (message[6])
    {
    case PROT_ID_CMD:
        sendIdentificationMessage(message);
        break;

    default:
        break;
    }
}


void TcpClient::sendSyncCommand()
{
    checkConnection();

    QByteArray syncData;
    syncData.append(static_cast<char>(0x00));
    syncData.append(static_cast<char>(0x80));
    CalculateCRC(syncData);

    QByteArray byteArray;
    byteArray.append(static_cast<char>(0xC0));
    byteArray.append(syncData);
    byteArray.append(static_cast<char>(0xC0));

    _currentMessage = byteArray;
    _socket.write(byteArray);

    emit dataSent(_currentMessage);

    // Reset Tx Rx
    _currTx = 0x00;
    _currRx = 0x00;
}


void TcpClient::sendIdentificationMessage(const QByteArray &message)
{
    checkConnection();

    UCHAR crc[2];

    // DATA
    // For now it's hardcoded data.
    FL_MODBUS_PROT_ID_CMD_MESSAGE idMessage;
    idMessage.protocol_version[0] = static_cast<UCHAR>(0x02);
    idMessage.protocol_version[1] = static_cast<UCHAR>(0x0A);
    idMessage.device_type = static_cast<UCHAR>(0x46);
    idMessage.validity = static_cast<UCHAR>(0x01);
    idMessage.config_version[0] = static_cast<UCHAR>(0xCE);
    idMessage.config_version[1] = static_cast<UCHAR>(0xCE);
    idMessage.firmware_version[0] = static_cast<UCHAR>(0x01);
    idMessage.firmware_version[1] = static_cast<UCHAR>(0x33);
    memset(idMessage.phone, 0, sizeof(idMessage.phone));
    memcpy(idMessage.phone, _phone.toUtf8().constData(), _phone.toUtf8().size());
    QByteArray rawData(reinterpret_cast<const char*>(&idMessage), sizeof(idMessage));
    QByteArray data(transformData(rawData));
    qDebug() << data;

    // HEADER
    FL_MODBUS_MESSAGE modbusMessage;
    modbusMessage.tx_id = message[0];
    modbusMessage.rx_id = _currRx + 0x01;
    modbusMessage.dist_addressMB = message[2];
    modbusMessage.FUNCT = message[3];
    modbusMessage.sour_address = message[5];
    modbusMessage.dist_address = message[4];
    modbusMessage.command = PROT_ID_OK;
    modbusMessage.len = static_cast<unsigned char>(data.size());
    QByteArray header(reinterpret_cast<const char*>(&modbusMessage), sizeof(modbusMessage));

    CalculateCRC(modbusMessage, data, crc);

    QByteArray byteArray;
    byteArray.append(static_cast<char>(0xC0));
    byteArray.append(header);
    byteArray.append(data);
    byteArray.append(crc[0]);
    byteArray.append(crc[1]);
    byteArray.append(static_cast<char>(0xC0));

    _currentMessage = byteArray;
    _socket.write(byteArray);

    // Update Tx Rx. For example, if we received Rx = 00, then we send Rx = 01 and wait for Rx = 01
    _currTx = modbusMessage.tx_id;
    _currRx = modbusMessage.rx_id;

    emit dataSent(_currentMessage);
}


QByteArray TcpClient::transformData(const QByteArray &input)
{
    QByteArray output;
    output.reserve(input.size() * 2);

    for (unsigned char byte : input)
    {
        if (byte == 0xC0)
        {
            output.append(static_cast<char>(0xDB));
            output.append(static_cast<char>(0xDC));
        }
        else if (byte == 0xDB)
        {
            output.append(static_cast<char>(0xDB));
            output.append(static_cast<char>(0xDD));
        }
        else
        {
            output.append(byte);
        }
    }

    return output;
}


void TcpClient::checkConnection()
{
    if (!_connected)
    {
        emit noConnection();
        return;
    }
}


void TcpClient::onSocketConnected()
{
    _connected = true;
    emit connected();
}


void TcpClient::onSocketDisconnected()
{
    _connected = false;
    emit disconnected();
}


void TcpClient::onSocketReadyRead()
{
    _receivedMessage = _socket.readAll();
    emit dataReceived(_receivedMessage);

    parseMessage(_receivedMessage);
}


void TcpClient::onSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QString errorString = _socket.errorString();

    emit errorOccurred(errorString);
}
