#include "device.h"
#include "Prot.h"

Device::Device(QObject *parent)
    : QObject{parent}
{
    connect(&_socket, &QTcpSocket::connected, this, &Device::onSocketConnected);
    connect(&_socket, &QTcpSocket::disconnected, this, &Device::onSocketDisconnected);
    connect(&_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
    connect(&_socket, &QTcpSocket::bytesWritten, this, &Device::onSocketBytesWritten);
    connect(&_socket, &QTcpSocket::readyRead, this, &Device::onSocketReadyRead);
}

Device::Device(const QString &phone, const QString &name, Logger *logger, QObject *parent)
    : Device{parent}
{
    _phone = phone;
    _name = name;
    setLogger(logger);
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

void Device::setLogger(Logger *logger)
{
    loggerInstance = logger;
}

void Device::connectToServer(const QString &serverAddress, quint16 serverPort)
{
    _socket.connectToHost(serverAddress, serverPort);
}

void Device::disconnectFromServer()
{
    _socket.disconnectFromHost();
}

void Device::sendIdentificationMessage()
{
    if (!_connected) {
        loggerInstance->logWarning(tr("Устройство не подключено к серверу!"));
        return;
    }

    FL_MODBUS_PROT_ID_CMD_MESSAGE idMessage;
    memset(&idMessage, 0, sizeof(idMessage));
    strcpy_s(idMessage.phone, _phone.toUtf8().constData());

    QByteArray byteArray;
    byteArray.append(reinterpret_cast<char*>(&idMessage), sizeof(idMessage));

    _currentMessage = byteArray;
    _socket.write(byteArray);
}

void Device::onSocketConnected()
{
    _connected = true;
    loggerInstance->logInfo(tr("Устройство с ID ") + _phone + tr(" подключено к серверу. Выполняется синхронизация..."));
    emit connected();

    sendSyncCommand();
}

void Device::onSocketDisconnected()
{
    _connected = false;
    loggerInstance->logInfo(tr("Устройство с ID ") + _phone + tr(" отключилось от сервера."));
    emit disconnected();
}

void Device::onSocketReadyRead()
{
    QByteArray receivedData = _socket.readAll();

    loggerInstance->logInfo(tr("Получено сообщение от сервера: ") + loggerInstance->byteArrToStr(receivedData));
}

void Device::onSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QString errorString = _socket.errorString();
    loggerInstance->logError(tr("Ошибка сокета: ") + errorString);
}

void Device::onSocketBytesWritten(qint64 bytes)
{
    if (bytes == _currentMessage.size()) {
        emit identificationMessageSent();
    }
}

void Device::sendSyncCommand()
{
    if (!_connected) {
        loggerInstance->logWarning(tr("Устройство не подключено к серверу!"));
        return;
    }

    std::vector<UCHAR> syncData;
    syncData.push_back(0x00);
    syncData.push_back(0x80);
    CalculateCRC(syncData);
    QByteArray byteArray;
    byteArray.append(static_cast<char>(0xC0));
    byteArray.append(reinterpret_cast<const char*>(syncData.data()), static_cast<int>(syncData.size()));
    byteArray.append(static_cast<char>(0xC0));

    _socket.write(byteArray);
    loggerInstance->logInfo(tr("Отправлено сообщение: ") + loggerInstance->byteArrToStr(byteArray));
}
