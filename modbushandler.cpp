#include "modbushandler.h"

ModbusHandler::ModbusHandler(QObject *parent)
    : QObject{parent},
    currentFileIterator(filesMap.begin()),
    currentFileInfo{}
{

}

void ModbusHandler::initModbusHandler(const QString &phone)
{
    _phone = phone;

    addState(0x21, QByteArray::fromHex("00000000000000000000000000000000"));
    addState(0x23, QByteArray::fromHex("80800000000000000000000000000000000000000000000000000000000000"));
    addState(0x25, QByteArray::fromHex("000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"));

    _myAddr = 0xD0;
    _serverAddr = 0x00;
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
            // Основной Кулон
            if (modbusMessage.dist_addressMB == 0x00 || modbusMessage.dist_addressMB == 0xD0)
                _myAddr = 0xD0;
            // Для файлов
            else if (modbusMessage.dist_addressMB == 0xDC)
                _myAddr = 0xDC;
            // Возможно придется поменять
            else
                _myAddr = modbusMessage.dist_addressMB;
            _serverAddr = modbusMessage.sour_address;

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
        // Пока что таких сообщений не приходило и обрабатывать их не умеем
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

    case PROT_RELAY_SET_CMD:
        setRelay(message);
        break;

    case PROT_FILE_SRCH_INIT_CMD:
        initFileSearch(message);
        break;

    case PROT_FILE_SRCH_CMD:
        searchFile(message);
        break;

    case PROT_FILE_RESULT_CMD:
        fileResult(true);
        break;

    case PROT_FILE_OPEN_RD_CMD:
        openReadFile(message);
        break;

    case PROT_FILE_RD_CMD:
        readFile(message);
        break;

    case PROT_FILE_CLOSE_CMD:
        closeFile(message);
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

void ModbusHandler::setRelay(const QByteArray &message)
{
    UCHAR relayByte = message[8];
    if (relayByte == static_cast<UCHAR>(0xFF))
    {
        QByteArray data = message.mid(8, 3);
        editRelayByte(data);
    }
    else
    {
        editRelayByte(relayByte);
    }
    formDefaultAnswer(message);
    formStateMessage(false);
}

void ModbusHandler::formDefaultAnswer(const QByteArray &message)
{
    UCHAR crc[2];
    QByteArray rawData{};
    FL_MODBUS_MESSAGE modbusMessage;
    modbusMessage.tx_id = _currTx + 0x01;
    modbusMessage.rx_id = _currRx + 0x01;
    modbusMessage.dist_addressMB = _myAddr;
    modbusMessage.FUNCT = message[3];
    modbusMessage.sour_address = _myAddr;
    modbusMessage.dist_address = _serverAddr;
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

    // DATA
    QByteArray data;
    for (const auto& message : stateMessage)
    {
        data.append(reinterpret_cast<const char*>(&message.len), sizeof(UCHAR));
        data.append(reinterpret_cast<const char*>(&message.type), sizeof(UCHAR));
        data.append(message.data);
    }
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

void ModbusHandler::addState(const UCHAR &type, const QByteArray &data)
{
    for (auto& state : stateMessage)
    {
        if (state.type == type) return;
    }
    FL_MODBUS_STATE_CMD_MESSAGE newState;
    newState.len = data.size() + 2;
    newState.type = type;
    newState.data = data;
    stateMessage.push_back(newState);
}

void ModbusHandler::initFileSearch(const QByteArray &message)
{
    // reset data
    currentFileIterator = filesMap.begin();
    currentFileInfo.clear();

    // Sends a regular answer
    formDefaultAnswer(message);
}

void ModbusHandler::searchFile(const QByteArray &message)
{
    QString fileNameTemplate = extractFileNameTemplate(message);
    QRegularExpression regex(fileNameTemplate.replace("*", ".*"));

    for (; currentFileIterator != filesMap.end(); ++currentFileIterator)
    {
        const QString &fileName = currentFileIterator.key();

        if (regex.match(fileName).hasMatch())
        {
            currentFileInfo.clear();
            currentFileInfo.append(static_cast<UCHAR>(PROT_FILE_NAME_EQ)); // имя совпадает с шаблоном
            currentFileInfo.append(QByteArray(5, '\0')); // зарезервировано
            int size = currentFileIterator.value().size();
            size = qToBigEndian(size);
            currentFileInfo.append(reinterpret_cast<const char*>(&size), sizeof(int));
            QByteArray dateArray = extractDateTime();
            currentFileInfo.append(dateArray); // дата и время
            currentFileInfo.append(fileName.toUtf8()); // имя файла
            currentFileInfo.append('\0');

            ++currentFileIterator;
            formDefaultAnswer(message);
            return;
        }
    }

    formDefaultAnswer(message);
//    currentFileIterator = filesMap.begin();
    currentFileInfo.clear();
}

void ModbusHandler::fileResult(bool calledAsResult)
{
    if (currentFileInfo.isEmpty())
    {
        replyError(PROT_ERR_NO_FILE);
        return;
    }

    UCHAR crc[2];

    // DATA
    QByteArray rawData(transformToRaw(currentFileInfo));

    // HEADER
    FL_MODBUS_MESSAGE modbusMessage;
    modbusMessage.tx_id = _currTx + 0x01;
    modbusMessage.rx_id = _currRx + 0x01;
    modbusMessage.dist_addressMB = _myAddr;
    modbusMessage.FUNCT = 0x6E;
    modbusMessage.sour_address = _myAddr;
    modbusMessage.dist_address = _serverAddr;
    if (calledAsResult)
        modbusMessage.command = PROT_FILE_RESULT_OK;
    else
        modbusMessage.command = PROT_FILE_OPEN_RD_OK;
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

void ModbusHandler::openReadFile(const QByteArray &message)
{
    QString fileName = extractFileNameTemplate(message);

    for(auto it = filesMap.begin(); it != filesMap.end(); ++it)
    {
        if(it.key() == fileName)
        {
            currentFileInfo.clear();
            currentFileInfo.append(static_cast<UCHAR>(PROT_FILE_NAME_EQ)); // имя совпадает с шаблоном
            currentFileInfo.append(QByteArray(5, '\0')); // зарезервировано
            int size = it.value().size();
            size = qToBigEndian(size);
            currentFileInfo.append(reinterpret_cast<const char*>(&size), sizeof(int));
            QByteArray dateArray = extractDateTime();
            currentFileInfo.append(dateArray); // дата и время
            currentFileInfo.append(fileName.toUtf8()); // имя файла
            currentFileInfo.append('\0');
            currentFileData.append(it.value());
            fileResult(false);
            return;
        }
    }

    // По идее если не нашли по каким то причинам файл надо кинуть ошибку
    replyError(PROT_ERR_NO_FILE);
}

void ModbusHandler::readFile(const QByteArray &message)
{
    if (endOfFile)
    {
        replyError(PROT_ERR_END_OF_FILE);
        return;
    }
    UCHAR crc[2];

    QByteArray messageData = message.mid(8, 5);


    quint32 offset = static_cast<quint32>((unsigned char)(messageData[0]) << 24 |
                                          (unsigned char)(messageData[1]) << 16 |
                                          (unsigned char)(messageData[2]) << 8  |
                                          (unsigned char)(messageData[3]));
    quint8 blockLength = static_cast<quint8>(messageData[4]);

    if (offset + blockLength >= currentFileData.size())
        endOfFile = true;

    // DATA
    QByteArray data;
    data.append(static_cast<char>((offset >> 24) & 0xFF));
    data.append(static_cast<char>((offset >> 16) & 0xFF));
    data.append(static_cast<char>((offset >> 8) & 0xFF));
    data.append(static_cast<char>(offset & 0xFF));
    data.append(currentFileData.mid(offset, blockLength));
    QByteArray rawData(transformToRaw(data));

    // HEADER
    FL_MODBUS_MESSAGE modbusMessage;
    modbusMessage.tx_id = _currTx + 0x01;
    modbusMessage.rx_id = _currRx + 0x01;
    modbusMessage.dist_addressMB = _myAddr;
    modbusMessage.FUNCT = 0x6E;
    modbusMessage.sour_address = _myAddr;
    modbusMessage.dist_address = _serverAddr;
    modbusMessage.command = PROT_FILE_RD_OK;
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

void ModbusHandler::closeFile(const QByteArray &message)
{
    endOfFile = false;
    currentFileData.clear();
    currentFileInfo.clear();
    formDefaultAnswer(message);
}

void ModbusHandler::replyError(UCHAR errorCode)
{
    UCHAR crc[2];

    QByteArray rawData;
    rawData.append(errorCode);

    // HEADER
    FL_MODBUS_MESSAGE modbusMessage;
    modbusMessage.tx_id = _currTx + 0x01;
    modbusMessage.rx_id = _currRx + 0x01;
    modbusMessage.dist_addressMB = _myAddr;
    modbusMessage.FUNCT = 0x6E;
    modbusMessage.sour_address = _myAddr;
    modbusMessage.dist_address = _serverAddr;
    modbusMessage.command = PROT_REPLY_ERROR;
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

void ModbusHandler::editRelayByte(UCHAR relayByte)
{
    int relayIndex = relayByte & 0x0F;
    bool turnRelayOn = (relayByte & 0xF0) >> 4;

    auto it = std::find_if(stateMessage.begin(), stateMessage.end(), [](const FL_MODBUS_STATE_CMD_MESSAGE& state) {
        return state.type == 0x21;
    });

    if (it != stateMessage.end())
    {
        if (turnRelayOn)
            it->data[0] |= (1 << relayIndex);
        else
            it->data[0] &= ~(1 << relayIndex);
    }
}

void ModbusHandler::editRelayByte(const QByteArray &relayMask)
{
    UCHAR relayState = relayMask[1];
    UCHAR relays = relayMask[2];

    auto it = std::find_if(stateMessage.begin(), stateMessage.end(), [](const FL_MODBUS_STATE_CMD_MESSAGE& state) {
        return state.type == 0x21;
    });

    if (it != stateMessage.end())
    {
        for (int i = 0; i < 4; ++i)
        {
            if (relays & (1 << i))
            {
                if (relayState & (1 << i))
                    it->data[0] |= (1 << i);
                else
                    it->data[0] &= ~(1 << i);
            }
        }
    }
}

void ModbusHandler::randomiseRelayStates()
{
    // first 4 bits
    uint8_t randomByte21 = QRandomGenerator::global()->bounded(16);
    // all 8 bits
    uint8_t randomByte23_1 = QRandomGenerator::global()->bounded(256);
    uint8_t randomByte23_2 = QRandomGenerator::global()->bounded(256);
    for (auto& state : stateMessage)
    {
        if (state.type == 0x21)
        {
            state.data[0] = randomByte21;
        }
        if (state.type == 0x23)
        {
            state.data[0] = randomByte23_1;
            state.data[1] = randomByte23_2;
        }
    }
}

void ModbusHandler::editByte(const UCHAR &stateByte, const QByteArray &byte)
{
    for (auto& state : stateMessage)
    {
        if (state.type == stateByte)
        {
            for (int i = 0; i < byte.size(); i++)
                state.data[i] = byte[i];
        }
    }
    formStateMessage(false);
}

void ModbusHandler::addFileToMap(const QString &fileName, const QByteArray &fileData)
{
    filesMap.insert(fileName, fileData);
    addState(0x08, QByteArray::fromHex("6400"));
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
                    i++;
                }
                else if (nextByte == char(0xDD))
                {
                    output.append(char(0xDB));
                    i++;
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

QString ModbusHandler::extractFileNameTemplate(const QByteArray &message)
{
    int nullIndex = message.indexOf('\0', 24);

    if (nullIndex != -1) {
        QByteArray templateBytes = message.mid(24, nullIndex - 24);
        return QString::fromUtf8(templateBytes);
    }

    return QString();
}

QByteArray ModbusHandler::extractDateTime()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    int year = dateTime.date().year();
    int month = dateTime.date().month();
    int day = dateTime.date().day();
    int hour = dateTime.time().hour();
    int minute = dateTime.time().minute();
    int second = dateTime.time().second();

    QByteArray result;
    result.append(static_cast<char>(year - 2000));
    result.append(static_cast<char>(month));
    result.append(static_cast<char>(day));
    result.append(static_cast<char>(hour));
    result.append(static_cast<char>(minute));
    result.append(static_cast<char>(second));

    return result;
}
