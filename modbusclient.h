#ifndef MODBUSCLIENT_H
#define MODBUSCLIENT_H

#include <QObject>
#include <QModbusTcpClient>
#include <QModbusDataUnit>

class ModbusClient : public QObject
{
    Q_OBJECT
public:
    explicit ModbusClient(QObject *parent = nullptr);
    ~ModbusClient();

    bool connectToServer(const QString &ip, quint16 port);
    bool sendIdentificationMessage(const QString &phone);

signals:
    void identificationMessageSent(bool success);

private:
    QModbusTcpClient *modbusDevice;
};

#endif // MODBUSCLIENT_H
