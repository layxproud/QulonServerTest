#include "tcpclient.h"

TcpClient::TcpClient(Logger* logger, const QString& phone, QObject *parent)
    : QObject{parent},
    _connected{false},
    _phone{phone},
    _logger{logger}
{
    connect(&_socket, &QTcpSocket::connected, this, &TcpClient::onSocketConnected);
    connect(&_socket, &QTcpSocket::disconnected, this, &TcpClient::onSocketDisconnected);
    connect(&_socket, &QAbstractSocket::errorOccurred, this, &TcpClient::onSocketError);
    connect(&_socket, &QTcpSocket::readyRead, this, &TcpClient::onSocketReadyRead);

    connect(&_modbusHandler, &ModbusHandler::messageToSend, this, &TcpClient::sendMessage);
    connect(&_modbusHandler, &ModbusHandler::wrongCRC, this, &TcpClient::onWrongCRC);
    connect(&_modbusHandler, &ModbusHandler::wrongTx, this, &TcpClient::onWrongTx);
    connect(&_modbusHandler, &ModbusHandler::unknownCommand, this, &TcpClient::onUnknownCommand);
    connect(&_modbusHandler, &ModbusHandler::replyError, this, &TcpClient::onReplyError);

    _modbusHandler.setPhone(_phone);
}


TcpClient::~TcpClient()
{
    if (_socket.state() == QAbstractSocket::ConnectedState)
        disconnectFromServer();
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

void TcpClient::sendState(const bool &outsideCall)
{
    _modbusHandler.formStateMessage(outsideCall);
}

void TcpClient::checkConnection()
{
    if (!_connected)
    {
        _logger->logWarning(tr("Устройство c ID ") + _phone + tr(" не подключено к серверу!"));
    }
}

void TcpClient::onSocketConnected()
{
    _connected = true;
    _logger->logInfo(tr("Устройство с ID ") + _phone + tr(" подключено к серверу. Выполняется синхронизация..."));
    emit connectionChanged(_connected);
}

void TcpClient::onSocketDisconnected()
{
    _connected = false;
    _logger->logInfo(tr("Устройство с ID ") + _phone + tr(" отключилось от сервера."));
    emit connectionChanged(_connected);
}

void TcpClient::onSocketReadyRead()
{
    _receivedMessage = _socket.readAll();
    _logger->logInfo(tr("ID ") + _phone + tr(" Получило сообщение: ") + _logger->byteArrToStr(_receivedMessage));

    _modbusHandler.parseMessage(_receivedMessage);
}

void TcpClient::onSocketError()
{
    QString errorString = _socket.errorString();
    _logger->logError(tr("Ошибка сокета: ") + errorString);
}

void TcpClient::onWrongCRC(const UCHAR &expected1, const UCHAR &received1, const UCHAR &expected2, const UCHAR &received2)
{
    QString message = tr("Неправильная контрольная сумма. Ожидалось: %1%2 | Получено: %3%4").arg(
        QString::number(expected1, 16).rightJustified(2, '0'),
        QString::number(expected2, 16).rightJustified(2, '0'),
        QString::number(received1, 16).rightJustified(2, '0'),
        QString::number(received2, 16).rightJustified(2, '0'));

    _logger->logError(message);
}

void TcpClient::onWrongTx(const UCHAR &expected, const UCHAR &received)
{
    QString message = tr("Tx не совпадают. Ожидалось: %1 | Получено: %2").arg(
        QString::number(expected, 16).rightJustified(2, '0'),
        QString::number(received, 16).rightJustified(2, '0'));

    _logger->logError(message);
}

void TcpClient::onUnknownCommand(const UCHAR &command)
{
    QString commandString = QString("0x%1").arg(command, 2, 16, QChar('0'));
    _logger->logWarning(tr("Встречена незнакомая команда: ") + commandString + tr(" Отправляю стандартный ответ..."));
}

void TcpClient::onReplyError()
{
    _logger->logError(tr("Сервер сообщил о возникшей ошибке"));
}

void TcpClient::sendMessage(const QByteArray &message)
{
    checkConnection();
    _currentMessage = message;
    _socket.write(message);

    _logger->logInfo(tr("ID ") + _phone + tr(" Отправило сообщение: ") + _logger->byteArrToStr(_currentMessage));
}
