#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include "Prot.h"

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);

    // Методы для подключения и отключения от сервера
    void connectToServer(const QString &serverAddress, quint16 serverPort);
    void disconnectFromServer();

    // Методы для отправки разных типов сообщений на сервер
    void sendSyncCommand();
    void sendIdentificationMessage(const QString &phone);

    // Метод для обработки входящих сообщений
    void parseMessage(const QByteArray &message);

signals:
    void connected();
    void disconnected();
    void dataSent(const QByteArray &data);
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &errorString);

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket _socket;
    QByteArray _currentMessage; // Буфер для отправленного сообщения
    QByteArray _receivedMessage; // Буфер для полученного сообщения
};

#endif // TCPCLIENT_H
