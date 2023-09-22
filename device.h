#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include "logger.h"
#include "tcpclient.h"

class Device : public QObject
{
    Q_OBJECT
public:
    explicit Device(QObject *parent = nullptr);
    Device(const QString &phone, const QString &name, Logger *logger, QObject *parent = nullptr);

    QString getPhone() const;
    QString getName() const;

    void connectToServer(const QString &serverAddress, quint16 serverPort);
    void disconnectFromServer();

public:
    TcpClient _client;

signals:
    void connected();
    void disconnected();

private slots:
    void onConnected();
    void onDisconnected();
    void onDataReceived(const QByteArray &data);
    void onDataSent(const QByteArray &data);
    void onError(const QString &errorString);
    void onNoConnection();

private:
    // ini file variables
    QString _phone;
    QString _name;

    // Logger
    Logger *loggerInstance;

private:
    void setLogger(Logger *logger);
};

#endif // DEVICE_H
