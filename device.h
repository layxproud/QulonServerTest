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
    bool isConnected() const;

    void connectToServer(const QString &serverAddress, quint16 serverPort);
    void disconnectFromServer();
    void sendIdentificationMessage();

signals:
    void connected();
    void disconnected();

private slots:
    void onConnected();
    void onDisconnected();
    void onDataReceived(const QByteArray &data);
    void onDataSent(const QByteArray &data);
    void onError(const QString &errorString);

private:
    // ini file variables
    QString _phone;
    QString _name;
    bool _connected = false;

    // Logger
    Logger *loggerInstance;

    // Network variables
    QByteArray _currentMessage; // Буфер для текущего сообщения
    TcpClient _client;

private:
    void setLogger(Logger *logger);

    void sendSyncCommand();
};

#endif // DEVICE_H
