#include "device.h"
#include "Prot.h"

Device::Device(QObject *parent)
    : QObject{parent}
{
    connect(&_socket, &QTcpSocket::connected, this, &Device::onSocketConnected);
    connect(&_socket, &QTcpSocket::disconnected, this, &Device::onSocketDisconnected);
    connect(&_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
    connect(&_socket, &QTcpSocket::bytesWritten, this, &Device::onSocketBytesWritten);
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

void Device::connectToServer(const QString &ip, quint16 port)
{
    _socket.connectToHost(ip, port);
    sendIdentificationMessage();
}

void Device::disconnectFromServer()
{
    _socket.disconnectFromHost();
}

void Device::sendIdentificationMessage()
{
    if (!_connected)
    {
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
    emit connected();
}

void Device::onSocketDisconnected()
{
    _connected = false;
    emit disconnected();
}

void Device::onSocketError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Socket error: " << socketError;
}

void Device::onSocketBytesWritten(qint64 bytes)
{
    if (bytes == _currentMessage.size())
    {
        emit identificationMessageSent();
    }
}
