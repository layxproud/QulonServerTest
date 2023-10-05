#include "tcpclient.h"

TcpClient::TcpClient(QObject *parent)
    : QObject{parent}, _connected{false}
{
    connect(&_socket, &QTcpSocket::connected, this, &TcpClient::onSocketConnected);
    connect(&_socket, &QTcpSocket::disconnected, this, &TcpClient::onSocketDisconnected);
    connect(&_socket, &QAbstractSocket::errorOccurred, this, &TcpClient::onSocketError);
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

    QList<QByteArray> messages = message.split(0xC0);

    for (const QByteArray &message : messages)
    {
        if (message.isEmpty())
            continue;

        // Sync message case
        if (message == syncMessage)
        {
            sendSyncCommand();
        }

        // FL_MODBUS_MESSAGE case
        else if (static_cast<unsigned char>(message[3]) == 0x6E)
        {
            // HEADER
            FL_MODBUS_MESSAGE modbusMessage;
            memcpy(&modbusMessage, message.constData(), sizeof(FL_MODBUS_MESSAGE));
            _serverAddr = modbusMessage.sour_address;
            _myAddr = modbusMessage.dist_address;

            // DATA
            int dataLength = modbusMessage.len;
            QByteArray data = message.mid(sizeof(FL_MODBUS_MESSAGE), dataLength);
            QByteArray rawData = transformToRaw(data);

            // CRC
            UCHAR crc[2];
            CalculateCRC(modbusMessage, rawData, crc);
            if ((static_cast<UCHAR>(crc[1]) != static_cast<UCHAR>(message.at(message.size() - 1))) &&
                (static_cast<UCHAR>(crc[0]) != static_cast<UCHAR>(message.at(message.size() - 2))))
            {
                emit wrongCRC(message.at(message.size() - 1), crc[1],
                              message.at(message.size() - 2), crc[0]);
                continue;
            }

            // Check Tx
            // Same message case
            if (_currTx == modbusMessage.tx_id)
            {
                _socket.write(_currentMessage);
                emit dataSent(_currentMessage);
            }
            // Next message case
            else if (_currTx + 0x01 == modbusMessage.tx_id)
            {
                performCommand(message);
            }
            // Wrong message
            else
            {
                emit wrongTx(_currTx, modbusMessage.tx_id);
            }
        }
        // FL_MODBUS_MESSAGE_SHORT case
        // TODO. I don't get the difference between MODBUS_MESSAGE and FL_MODBUS_MESSAGE_SHORT
    }
}


void TcpClient::performCommand(const QByteArray &message)
{
    switch (message[6])
    {
    case PROT_ID_CMD:
        sendIdentificationMessage();
        break;

    case PROT_STATE_REQ_CMD:
        sendState(false);
        break;

    case PROT_REPLY_ERROR:
        emit replyError();
        break;

    default:
        emit unknownCommand(message[6]);
        sendDefaultResponce(message);
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
    _currTx = 0x80;
    _currRx = 0x00;
}


void TcpClient::sendIdentificationMessage()
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
    QByteArray data(transformToData(rawData));

    // HEADER
    FL_MODBUS_MESSAGE modbusMessage;
    modbusMessage.tx_id = _currTx + 0x01;
    modbusMessage.rx_id = _currRx + 0x01;
    modbusMessage.dist_addressMB = _myAddr;
    modbusMessage.FUNCT = 0x6E;
    modbusMessage.sour_address = _myAddr;
    modbusMessage.dist_address = _serverAddr;
    modbusMessage.command = PROT_ID_OK;
    modbusMessage.len = static_cast<unsigned char>(data.size());
    QByteArray header(reinterpret_cast<const char*>(&modbusMessage), sizeof(modbusMessage));

    // CRC
    CalculateCRC(modbusMessage, rawData, crc);

    QByteArray byteArray;
    byteArray.append(static_cast<char>(0xC0));
    byteArray.append(header);
    byteArray.append(data);
    byteArray.append(crc[0]);
    byteArray.append(crc[1]);
    byteArray.append(static_cast<char>(0xC0));

    _currentMessage = byteArray;
    _socket.write(byteArray);

    // Update Tx Rx
    _currTx = modbusMessage.tx_id;
    _currRx = modbusMessage.rx_id;

    emit dataSent(_currentMessage);
}

void TcpClient::sendState(const bool &outsideCall)
{
    checkConnection();

    UCHAR crc[2];

    // DATA (Hardcode for now)
    QByteArray data = QByteArray::fromHex("390151756C6F6E2D43322D5363656E322C2049503A203139322E3136382E312E36342028455448292C20636F6E6E656374206661696C656400040400630302000324001221000000000000000000000000000000002123808000000000000000000000000000000000000000000000000000000000002125000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF032600");
    quint8 randomValue = QRandomGenerator::global()->bounded(16);
    data[69] = randomValue;
    randomValue = QRandomGenerator::global()->bounded(256);
    data[87] = randomValue;
    QByteArray rawData(transformToRaw(data));

    // HEADER
    FL_MODBUS_MESSAGE modbusMessage;
    if (outsideCall)
        modbusMessage.tx_id = _currTx;
    else
        modbusMessage.tx_id = _currTx + 0x01;
    modbusMessage.rx_id = _currRx + 0x01;
    modbusMessage.dist_addressMB = _myAddr;
    modbusMessage.FUNCT = 0x6E;
    modbusMessage.sour_address = _myAddr;
    modbusMessage.dist_address = _serverAddr;
    modbusMessage.command = PROT_STATE_REQ_OK;
    modbusMessage.len = static_cast<unsigned char>(data.size());
    QByteArray header(reinterpret_cast<const char*>(&modbusMessage), sizeof(modbusMessage));

    // CRC
    CalculateCRC(modbusMessage, rawData, crc);

    QByteArray byteArray;
    byteArray.append(static_cast<char>(0xC0));
    byteArray.append(header);
    byteArray.append(data);
    byteArray.append(crc[0]);
    byteArray.append(crc[1]);
    byteArray.append(static_cast<char>(0xC0));

    _currentMessage = byteArray;
    _socket.write(byteArray);

    // Update Tx Rx
    _currTx = modbusMessage.tx_id;
    _currRx = modbusMessage.rx_id;

    emit dataSent(_currentMessage);
}


void TcpClient::sendDefaultResponce(const QByteArray &message)
{
    checkConnection();

    UCHAR crc[2];
    QByteArray data{};
    FL_MODBUS_MESSAGE modbusMessage;
    modbusMessage.tx_id = _currTx + 0x01;
    modbusMessage.rx_id = _currRx + 0x01;
    modbusMessage.dist_addressMB = message[2];
    modbusMessage.FUNCT = message[3];
    modbusMessage.sour_address = message[5];
    modbusMessage.dist_address = message[4];
    modbusMessage.command = message[6] + 0x80;
    modbusMessage.len = 0x00;
    QByteArray header(reinterpret_cast<const char*>(&modbusMessage), sizeof(modbusMessage));

    // CRC
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

    _currTx = modbusMessage.tx_id;
    _currRx = modbusMessage.rx_id;

    emit dataSent(_currentMessage);

}


QByteArray TcpClient::transformToData(const QByteArray &input)
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


QByteArray TcpClient::transformToRaw(const QByteArray &message)
{
    QByteArray output;
    output.reserve(message.size());

    for (int i = 0; i < message.size(); ++i)
    {
        char currentByte = message.at(i);

        if (currentByte == char(0xDB))
        {
            if (i + 1 < message.size())
            {
                char nextByte = message.at(i + 1);
                if (nextByte == char(0xDC))
                {
                    output.append(char(0xC0));
                    i++; // Skip the next byte
                }
                else if (nextByte == char(0xDD))
                {
                    output.append(char(0xDB));
                    i++; // Skip the next byte
                }
                else
                {
                    output.append(currentByte);
                }
            }
            else
            {
                output.append(currentByte);
            }
        }
        else
        {
            output.append(currentByte);
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
    emit connectionChanged(_connected);
}


void TcpClient::onSocketDisconnected()
{
    _connected = false;
    emit connectionChanged(_connected);
}


void TcpClient::onSocketReadyRead()
{
    _receivedMessage = _socket.readAll();
    emit dataReceived(_receivedMessage);

    parseMessage(_receivedMessage);
}


void TcpClient::onSocketError()
{
    QString errorString = _socket.errorString();
    emit errorOccurred(errorString);
}
