#include "device.h"
#include "Prot.h"

Device::Device(QObject *parent)
    : QObject{parent}
{
    connect(&_socket, &QTcpSocket::connected, this, &Device::onSocketConnected);
    connect(&_socket, &QTcpSocket::disconnected, this, &Device::onSocketDisconnected);
//    connect(&_socket, &QTcpSocket::stateChanged, this, [](QAbstractSocket::SocketState state) {
//        if (state == QAbstractSocket::ConnectedState) {
//            qDebug() << "Modbus connection established";
//        } else if (state == QAbstractSocket::ConnectingState) {
//            qDebug() << "Modbus connecting...";
//        } else if (state == QAbstractSocket::UnconnectedState) {
//            qDebug() << "Modbus disconnected";
//        }
//    });
    connect(&_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
    connect(&_socket, &QTcpSocket::bytesWritten, this, &Device::onSocketBytesWritten);
    connect(&_socket, &QTcpSocket::readyRead, this, &Device::onSocketReadyRead);
}

Device::Device(const QString &phone, const QString &name, QObject *parent)
    : Device{parent}
{
    _phone = phone;
    _name = name;
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
        qDebug() << "Device is not connected to the server.";
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
    qDebug() << "Connected";
    emit connected();

    sendSyncCommand();
}

void Device::onSocketDisconnected()
{
    _connected = false;
    qDebug() << "Disconnected";
    emit disconnected();
}

void Device::onSocketReadyRead()
{
    QByteArray receivedData = _socket.readAll();

    qDebug() << "Received data from server:" << receivedData;
}

void Device::onSocketError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Socket error: " << socketError;
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
        qDebug() << "Device is not connected to the server.";
        return;
    }

    // Создаем объект mm и инициализируем его значениями
    FL_MODBUS_MESSAGE mm;
    mm.tx_id = 0x00;
    mm.rx_id = 0x00;
    mm.dist_addressMB = 0x00;
    mm.FUNCT = 0x10;

    // Вычисляем CRC для синхронизационного сообщения
    UCHAR crc[2];
    CalculateCRC(mm, std::vector<UCHAR>(), crc);
    qDebug() << crc;

    // QByteArray byteArray;
    // byteArray.append(reinterpret_cast<char*>(&mm), sizeof(mm));

    // _socket.write(byteArray);
    // qDebug() << "Trying to send message: " << byteArray;
}
