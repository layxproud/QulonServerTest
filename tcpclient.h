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

    void connectToServer(const QString &serverAddress, quint16 serverPort);
    void disconnectFromServer();

    void parseMessage(const QByteArray &message);

private:
    QString _phone;
    QTcpSocket _socket;
    bool _connected = false;
    QByteArray _currentMessage;
    QByteArray _receivedMessage;
    UCHAR _currTx;
    UCHAR _currRx;

private:
    void performCommand(const QByteArray &message, const QByteArray &data);
    void sendSyncCommand();
    void sendIdentificationMessage(const QByteArray &message);

    QByteArray transformData(const QByteArray &input);
    void checkConnection();

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketError(QAbstractSocket::SocketError socketError);

signals:
    void connected();
    void disconnected();
    void dataSent(const QByteArray &data);
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &errorString);
    void noConnection();
    void wrongCRC(const UCHAR &expected1, const UCHAR &received1,
                  const UCHAR &expected2, const UCHAR &received2);
    void wrongRx(const UCHAR &expected, const UCHAR &received);

};

#endif // TCPCLIENT_H
