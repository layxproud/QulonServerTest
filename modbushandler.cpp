#include "modbushandler.h"

ModbusHandler::ModbusHandler(QObject *parent)
    : QObject{parent}
{

}

void ModbusHandler::setPhone(const QString &phone)
{
    _phone = phone;
}

void ModbusHandler::parseMessage(const QByteArray &message)
{
    _receivedMessage = message;
    QList<QByteArray> messages = message.split(0xC0);

    for (const QByteArray &message : messages)
    {
        QByteArray rawMessage = transformToRaw(message);
        if (rawMessage.isEmpty())
            continue;

        // Sync message case
        if (rawMessage == SYNC_MESSAGE)
        {
            formSyncMessage();
        }

        // FL_MODBUS_MESSAGE case
        else if (static_cast<unsigned char>(rawMessage[3]) == 0x6E)
        {
            // HEADER
            FL_MODBUS_MESSAGE modbusMessage;
            memcpy(&modbusMessage, rawMessage.constData(), sizeof(FL_MODBUS_MESSAGE));
            _serverAddr = modbusMessage.sour_address;
            _myAddr = modbusMessage.dist_address;

            // DATA
            int dataLength = modbusMessage.len;
            QByteArray rawData = rawMessage.mid(sizeof(FL_MODBUS_MESSAGE), dataLength);

            // CRC
            UCHAR crc[2];
            CalculateCRC(modbusMessage, rawData, crc);
            if ((static_cast<UCHAR>(crc[1]) != static_cast<UCHAR>(rawMessage.at(rawMessage.size() - 1))) &&
                (static_cast<UCHAR>(crc[0]) != static_cast<UCHAR>(rawMessage.at(rawMessage.size() - 2))))
            {
                emit wrongCRC(rawMessage.at(rawMessage.size() - 1), crc[1],
                              rawMessage.at(rawMessage.size() - 2), crc[0]);
                continue;
            }

            // Check Tx
            // Same message case
            if (_currTx == modbusMessage.tx_id)
            {
                emit messageToSend(_currentMessage);
            }
            // Next message case
            else if (_currTx + 0x01 == modbusMessage.tx_id)
            {
                performCommand(rawMessage);
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

void ModbusHandler::performCommand(const QByteArray &message)
{
    switch (message[6])
    {
    case PROT_ID_CMD:
        formIdentificationMessage();
        break;

    case PROT_STATE_REQ_CMD:
        formStateMessage(false);
        break;

    case PROT_REPLY_ERROR:
        emit replyError();
        break;

    default:
        emit unknownCommand(message[6]);
        formDefaultAnswer(message);
        break;
    }
}

void ModbusHandler::formSyncMessage()
{
    QByteArray syncData;
    syncData.append(static_cast<char>(0x00));
    syncData.append(static_cast<char>(0x80));
    CalculateCRC(syncData);

    QByteArray byteArray;
    byteArray.append(static_cast<char>(0xC0));
    byteArray.append(syncData);
    byteArray.append(static_cast<char>(0xC0));

    _currentMessage = byteArray;

    // Reset Tx Rx
    _currTx = 0x80;
    _currRx = 0x00;

    emit messageToSend(_currentMessage);
}

void ModbusHandler::formDefaultAnswer(const QByteArray &message)
{
    UCHAR crc[2];
    QByteArray rawData{};
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
    CalculateCRC(modbusMessage, rawData, crc);

    // Transform to modbus protocol
    QByteArray messageArray;
    messageArray.append(header);
    messageArray.append(rawData);
    messageArray.append(crc[0]);
    messageArray.append(crc[1]);
    messageArray = transformToData(messageArray);
    QByteArray byteArray = addMarkerBytes(messageArray);

    _currentMessage = byteArray;
    _currTx = modbusMessage.tx_id;
    _currRx = modbusMessage.rx_id;

    emit messageToSend(byteArray);
}

void ModbusHandler::formStateMessage(const bool &outsideCall)
{
    UCHAR crc[2];

    // DATA (Hardcode for now)
    FL_MODBUS_STATE_CMD_MESSAGE stateMessage;
    stateMessage.header = QByteArray::fromHex("390151756C6F6E2D43322D5363656E322C2049503A203139322E3136382E312E36342028455448292C20636F6E6E656374206661696C6564000404006303");
    stateMessage.state02 = QByteArray::fromHex("020003");
    stateMessage.state24 = QByteArray::fromHex("240012");
    stateMessage.state21 = QByteArray::fromHex("210000000000000000000000000000000021");
    stateMessage.state23 = QByteArray::fromHex("238080000000000000000000000000000000000000000000000000000000000021");
    stateMessage.state25 = QByteArray::fromHex("25000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF03");
    stateMessage.state26 = QByteArray::fromHex("2600");
    quint8 randomValue = QRandomGenerator::global()->bounded(16);
    stateMessage.state21[1] = randomValue;
    randomValue = QRandomGenerator::global()->bounded(256);
    stateMessage.state23[1] = randomValue;
    QByteArray data;
    data.append(stateMessage.header);
    data.append(stateMessage.state02);
    data.append(stateMessage.state24);
    data.append(stateMessage.state21);
    data.append(stateMessage.state23);
    data.append(stateMessage.state25);
    data.append(stateMessage.state26);
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

    // Transform to modbus protocol
    QByteArray messageArray;
    messageArray.append(header);
    messageArray.append(rawData);
    messageArray.append(crc[0]);
    messageArray.append(crc[1]);
    messageArray = transformToData(messageArray);
    QByteArray byteArray = addMarkerBytes(messageArray);

    _currentMessage = byteArray;
    _currTx = modbusMessage.tx_id;
    _currRx = modbusMessage.rx_id;

    emit messageToSend(byteArray);
}

void ModbusHandler::formIdentificationMessage()
{
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

    // HEADER
    FL_MODBUS_MESSAGE modbusMessage;
    modbusMessage.tx_id = _currTx + 0x01;
    modbusMessage.rx_id = _currRx + 0x01;
    modbusMessage.dist_addressMB = _myAddr;
    modbusMessage.FUNCT = 0x6E;
    modbusMessage.sour_address = _myAddr;
    modbusMessage.dist_address = _serverAddr;
    modbusMessage.command = PROT_ID_OK;
    modbusMessage.len = static_cast<unsigned char>(rawData.size());
    QByteArray header(reinterpret_cast<const char*>(&modbusMessage), sizeof(modbusMessage));

    // CRC
    CalculateCRC(modbusMessage, rawData, crc);

    // Transform to modbus protocol
    QByteArray messageArray;
    messageArray.append(header);
    messageArray.append(rawData);
    messageArray.append(crc[0]);
    messageArray.append(crc[1]);
    messageArray = transformToData(messageArray);
    QByteArray byteArray = addMarkerBytes(messageArray);

    _currentMessage = byteArray;
    _currTx = modbusMessage.tx_id;
    _currRx = modbusMessage.rx_id;

    emit messageToSend(byteArray);
}

QByteArray ModbusHandler::addMarkerBytes(const QByteArray &input)
{
    QByteArray output;
    output.append(static_cast<char>(0xC0));
    output.append(input);
    output.append(static_cast<char>(0xC0));
    return output;
}

QByteArray ModbusHandler::transformToData(const QByteArray& message)
{
    QByteArray output;
    output.reserve(message.size() * 2);

    for (unsigned char byte : message)
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

QByteArray ModbusHandler::transformToRaw(const QByteArray& message)
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
