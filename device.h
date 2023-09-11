#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QTcpSocket>

class Device : public QObject
{
    Q_OBJECT
public:
    explicit Device(QObject *parent = nullptr);
    Device(const QString &phone, const QString &name, QObject *parent = nullptr);

    QString getPhone() const;
    QString getName() const;
    bool isConnected() const;

    void connectToServer(const QString &ip, quint16 port);
    void disconnectFromServer();
    void sendIdentificationMessage();

signals:
    void connected();
    void disconnected();
    void identificationMessageSent();

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError socketError);
    void onSocketBytesWritten(qint64 bytes);

private:
    QString _phone;
    QString _name;
    bool _connected = false;
    QTcpSocket _socket;

    QByteArray _currentMessage;

};

#endif // DEVICE_H
