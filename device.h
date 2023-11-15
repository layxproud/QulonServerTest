#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QTimer>
#include "logger.h"
#include "tcpclient.h"

struct DeviceDefaults
{
    int connectionInterval = 60000;
    int disconnectionFromInterval = 300000;
    int disconnectionToInterval = 600000;
    int sendStatusInterval = 30000;
    int changeStatusInterval = 30000;
    bool autoRegen = true;
    bool logStatus = true;
};

class Device : public QObject
{
    Q_OBJECT
public:
    explicit Device(QObject *parent = nullptr);
    Device(const QString &phone, const QString &name, Logger *logger, QObject *parent = nullptr);
    ~Device();

    // Getters
    QString getPhone() const;
    QString getName() const;
    bool isConnected() const;

    // Setters
    void setIp(const QString &ip);
    void setPort(const quint16 &port);
    void setAutoRegen(const bool &regen);
    void setDefaults(const DeviceDefaults& defaults);

    void startWork();
    void stopWork();

    void debugConnect(const QString &serverAddress, quint16 serverPort);
    void editByte(const UCHAR &stateByte, const QByteArray &byte);
    void editLogStatus(const bool &status);

private:
    QString _phone;
    QString _name;
    QString _ip;
    quint16 _port;
    int _phoneId;
    bool _connected;
    bool _autoRegen;
    DeviceDefaults _defaults;

    Logger* _logger;
    TcpClient* _client;

    // Таймеры
    QTimer *connectionTimer;
    QTimer *disconnectionTimer;
    QTimer *sendStatusTimer;
    QTimer *changeStatusTimer;

private:
    // Таймеры
    void setConnectionInterval(const int &interval);
    void setDisconnectionInterval(const int &from, const int &to);
    void setSendStatusInterval(const int &interval);
    void setChangeStatusInterval(const int &interval);

    void startConnectionTimer();
    void startDisconnectionTimer();
    void startSendStatusTimer();
    void startChangeStatusTimer();

private slots:
    void onConnectionChanged(const bool &status);
    void onConnectionTimerTimeout();
    void onDisconnectionTimerTimeout();
    void onSendStatusTimerTimeout();
    void onChangeStatusTimeTimeout();

signals:
    void connectionChanged(const bool &status);
};

#endif // DEVICE_H
