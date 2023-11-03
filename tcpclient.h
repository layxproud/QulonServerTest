#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include "modbushandler.h"
#include "logger.h"

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(Logger* logger, const QString &phone, QObject *parent = nullptr);
    ~TcpClient();

    bool isConnected() const;
    void connectToServer(const QString &serverAddress, quint16 serverPort);
    void disconnectFromServer();
    void sendState(const bool &outsideCall);
    void randomiseState();
    void editByte(const UCHAR &stateByte, const QByteArray &byte);
    void editLogStatus(const bool &status);

private:
    QString _phone;
    QTcpSocket _socket;
    bool _connected;
    QByteArray _currentMessage;
    QByteArray _receivedMessage;
    ModbusHandler _modbusHandler;
    bool _logAllowed;

    Logger* _logger;

private:

    void sendDefaultResponce(const QByteArray &message);

    QByteArray transformToData(const QByteArray &input);
    QByteArray transformToRaw(const QByteArray &input);
    bool checkConnection();

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketError();
    void onWrongCRC(const UCHAR &expected1, const UCHAR &received1,
                    const UCHAR &expected2, const UCHAR &received2);
    void onWrongTx(const UCHAR &expected, const UCHAR &received);
    void onUnknownCommand(const UCHAR &command);
    void onReplyError();

    void sendMessage(const QByteArray& message);

signals:
    void connectionChanged(const bool &status);
};

#endif // TCPCLIENT_H
