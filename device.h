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
    void setDisconnectionInterval(const int &from, const int &to);
    void setSendStatusInterval(const int &interval);

    void startWork();
    void stopWork();

public:
    TcpClient _client;

private:
    QString _phone;
    QString _name;
    QString _ip;
    quint16 _port;
    int _phoneId;

    Logger *_logger;

    QTimer *connectionTimer;
    QTimer *disconnectionTimer;
    QTimer *statusTimer;

    // default values
    int _connectionInterval = 60000;
    int _disconnectionFromInterval = 300000;
    int _disconnectionToInterval = 600000;
    int _sendStatusInterval = 30000;

private:
    int phoneToId();
    void startConnectionTimer();
    void startDisconnectionTimer();
    void startStatusTimer();

private slots:
    void onConnectionChanged(const bool &status);
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
    void onStatusTimerTimeout();

signals:
    void connectionChanged(const bool &status);
};

#endif // DEVICE_H
