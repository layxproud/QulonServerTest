#ifndef MODBUSHANDLER_H
#define MODBUSHANDLER_H

#include <QObject>
#include "Prot.h"

class ModbusHandler : public QObject
{
    Q_OBJECT
public:
    explicit ModbusHandler(QObject *parent = nullptr);
    void setPhone(const QString& phone);
    void parseMessage(const QByteArray &rawMessage);
    void formStateMessage(const bool &outsideCall);

private:
    const QByteArray SYNC_MESSAGE = QByteArray::fromHex("00800010");

    QString _phone;
    UCHAR _currTx;
    UCHAR _currRx;
    UCHAR _myAddr;
    UCHAR _serverAddr;

    QByteArray _currentMessage;
    QByteArray _receivedMessage;

private:
    void performCommand(const QByteArray &message);
    void formSyncMessage();
    void formDefaultAnswer(const QByteArray& message);
    void formIdentificationMessage();

    QByteArray addMarkerBytes(const QByteArray& input);
    QByteArray transformToData(const QByteArray& input);
    QByteArray transformToRaw(const QByteArray& message);

signals:
    QByteArray messageToSend(const QByteArray& message);
    void wrongCRC(const UCHAR &expected1, const UCHAR &received1,
                  const UCHAR &expected2, const UCHAR &received2);
    void wrongTx(const UCHAR &expected, const UCHAR &received);
    void unknownCommand(const UCHAR &command);
    void replyError();
};

#endif // MODBUSHANDLER_H
