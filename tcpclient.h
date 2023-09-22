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
    ~TcpClient();

    void setPhone(const QString &phone);
    bool isConnected() const;

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
    void noConnection();

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketError(QAbstractSocket::SocketError socketError);

private:
    QString _phone;
    QTcpSocket _socket;
    bool _connected = false;
    QByteArray _currentMessage; // Буфер для отправленного сообщения
    QByteArray _receivedMessage; // Буфер для полученного сообщения
    UCHAR _currTx;
    UCHAR _currRx;

private:
    void checkConnection();
    QByteArray transformData(const QByteArray &input);
};

#endif // TCPCLIENT_H
