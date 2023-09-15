#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QTcpSocket>
#include "logger.h"

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
    void identificationMessageSent();

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketError(QAbstractSocket::SocketError socketError);
    void onSocketBytesWritten(qint64 bytes);

private:
    // ini file variables
    QString _phone;
    QString _name;
    bool _connected = false;

    // Logger
    Logger *loggerInstance;

    // Network variables
    QTcpSocket _socket;
    QByteArray _currentMessage; // Буфер для текущего сообщения

private:
    void setLogger(Logger *logger);

    void sendSyncCommand();
};

#endif // DEVICE_H
