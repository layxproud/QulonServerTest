#include "modbusclient.h"
#include <QDebug>
#include <QVariant>

ModbusClient::ModbusClient(QObject *parent)
    : QObject{parent}
{
    modbusDevice = new QModbusTcpClient(this);
    connect(modbusDevice, &QModbusClient::stateChanged, this, [](QModbusDevice::State state) {
        if (state == QModbusDevice::ConnectedState) {
            qDebug() << "Modbus connection established";
        } else if (state == QModbusDevice::ConnectingState) {
            qDebug() << "Modbus connecting...";
        } else if (state == QModbusDevice::UnconnectedState) {
            qDebug() << "Modbus disconnected";
        }
    });
}

ModbusClient::~ModbusClient()
{
    if (modbusDevice)
    {
        modbusDevice->disconnectDevice();
        delete modbusDevice;
    }
}

bool ModbusClient::connectToServer(const QString &ip, quint16 port)
{
    modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, QVariant(ip));
    modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, QVariant(port));

    if (!modbusDevice->connectDevice())
    {
        return false;
    }

    return true;
}

bool ModbusClient::sendIdentificationMessage(const QString &phone)
{
    if (modbusDevice->state() != QModbusDevice::ConnectedState)
    {
        return false;
    }

    QModbusDataUnit writeRequest(QModbusDataUnit::HoldingRegisters, 0, 1); // Adjust address and count as needed
    writeRequest.setValue(0, phone.toInt());

    auto *reply = modbusDevice->sendWriteRequest(writeRequest, 1); // Slave address is 1

    if (!reply)
    {
        return false;
    }

    QObject::connect(reply, &QModbusReply::finished, this, [this, reply]() {
        if (reply->error() == QModbusDevice::NoError)
        {
            emit identificationMessageSent(true);
        }
        else
        {
            emit identificationMessageSent(false);
        }

        reply->deleteLater();
    });

    return true;
}
