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

    void parseMessage(const QByteArray &rawMessage);

    void sendState(const bool &outsideCall);

private:
    QString _phone;
    QTcpSocket _socket;
    bool _connected;
    QByteArray _currentMessage;
    QByteArray _receivedMessage;
    UCHAR _currTx;
    UCHAR _currRx;
    UCHAR _myAddr;
    UCHAR _serverAddr;

private:
    void performCommand(const QByteArray &message);
    void sendSyncCommand();
    void sendIdentificationMessage();
    void sendDefaultResponce(const QByteArray &message);

    QByteArray transformToData(const QByteArray &input);
    QByteArray transformToRaw(const QByteArray &input);
    void checkConnection();

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketError();

signals:
    void connectionChanged(const bool &status);
    void dataSent(const QByteArray &data);
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &errorString);
    void unknownCommand(const UCHAR &command);
    void replyError();
    void noConnection();
    void wrongCRC(const UCHAR &expected1, const UCHAR &received1,
                  const UCHAR &expected2, const UCHAR &received2);
    void wrongTx(const UCHAR &expected, const UCHAR &received);

};

#endif // TCPCLIENT_H
