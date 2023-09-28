#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QTimer>
#include "logger.h"
#include "tcpclient.h"

class Device : public QObject
{
    Q_OBJECT
public:
    explicit Device(QObject *parent = nullptr);
    Device(const QString &phone, const QString &name, Logger *logger, QObject *parent = nullptr);
    ~Device();

    QString getPhone() const;
    QString getName() const;

    void setIp(const QString &ip);
    void setPort(const quint16 &port);
    void setConnectionInterval(const int &interval);
    void setDisconnectionInterval(const int &interval);

    void startConnectionTimer();
    void startDisconnectionTimer();

public:
    TcpClient _client;

private:
    QString _phone;
    QString _name;
    QString _ip;
    quint16 _port;
    int _phoneId;

    Logger *loggerInstance;

    QTimer *connectionTimer;
    QTimer *disconnectionTimer;
    int _connectionInterval = 1;
    int _disconnectionInterval = 20;

private:
    void setLogger(Logger *logger);
    int phoneToId();

private slots:
    void onConnected();
    void onDisconnected();
    void onDataReceived(const QByteArray &data);
    void onDataSent(const QByteArray &data);
    void onError(const QString &errorString);
    void onUnknownCommand(const UCHAR &command);
    void onNoConnection();
    void onReplyError();
    void onWrongCRC(const UCHAR &expected1, const UCHAR &received1,
                    const UCHAR &expected2, const UCHAR &received2);
    void onWrongTx(const UCHAR &expected, const UCHAR &received);

    void onConnectionTimerTimeout();
    void onDisconnectionTimerTimeout();

signals:
    void connected();
    void disconnected();
};

#endif // DEVICE_H
