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
    bool isConnected() const;

    void setIp(const QString &ip);
    void setPort(const quint16 &port);
    void setConnectionInterval(const int &interval);
    void setDisconnectionInterval(const int &from, const int &to);
    void setSendStatusInterval(const int &interval);

    void startWork();
    void stopWork();

    void debugConnect(const QString& serverAddress, quint16 serverPort);

private:
    QString _phone;
    QString _name;
    QString _ip;
    quint16 _port;
    int _phoneId;
    bool _connected;

    Logger* _logger;
    TcpClient* _client;

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

    void onConnectionTimerTimeout();
    void onDisconnectionTimerTimeout();
    void onStatusTimerTimeout();

signals:
    void connectionChanged(const bool &status);
};

#endif // DEVICE_H
