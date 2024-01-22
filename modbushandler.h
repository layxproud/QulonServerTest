#ifndef MODBUSHANDLER_H
#define MODBUSHANDLER_H

#include <QObject>
#include <QRegularExpression>
#include <QMap>
#include <QDateTime>
#include "Prot.h"

class ModbusHandler : public QObject
{
    Q_OBJECT
public:
    explicit ModbusHandler(QObject *parent = nullptr);
    void initModbusHandler(const QString& phone);
    void parseMessage(const QByteArray &rawMessage);
    void formStateMessage(const bool &outsideCall);
    void randomiseRelayStates();
    void editByte(const UCHAR &stateByte, const QByteArray &byte);
    void addFileToMap(const QString &fileName, const QByteArray &fileData);
    void editAhpState(const QByteArray &data);

private:
    const QByteArray SYNC_MESSAGE = QByteArray::fromHex("00800010");

    QString _phone;
    UCHAR _currTx;
    UCHAR _currRx;
    UCHAR _myAddr;
    UCHAR _serverAddr;

    QByteArray _currentMessage;
    QByteArray _receivedMessage;

    // Stores relay state. Has default states, new can be added
    std::vector<FL_MODBUS_STATE_CMD_MESSAGE> stateMessage;
    // Map for storing all virtual files.
    QMap<QString, QByteArray> filesMap;
    // Map's iterator to store current file.
    QMap<QString, QByteArray>::iterator currentFileIterator;
    // Stores current file
    QByteArray currentFileInfo;
    QByteArray currentFileData;

    bool endOfFile = false;

private:
    void performCommand(const QByteArray &message);
    void formSyncMessage();
    void setRelay(const QByteArray &message);
    void formDefaultAnswer(const QByteArray &message);
    void formIdentificationMessage();
    void addState(const UCHAR &type, const QByteArray &data);
    void initFileSearch(const QByteArray &message);
    void searchFile(const QByteArray &message);
    void fileResult(bool calledAsResult);
    void openReadFile(const QByteArray &message);
    void readFile(const QByteArray &message);
    void closeFile(const QByteArray &message);
    void replyError(UCHAR errorCode);

    void editRelayByte(UCHAR relayByte);
    void editRelayByte(const QByteArray &relayMask);

    QByteArray addMarkerBytes(const QByteArray& input);
    QByteArray transformToData(const QByteArray& input);
    QByteArray transformToRaw(const QByteArray& message);
    QString extractFileNameTemplate(const QByteArray &message);
    QByteArray extractDateTime();

signals:
    QByteArray messageToSend(const QByteArray& message);
    void wrongCRC(const UCHAR &expected1, const UCHAR &received1,
                  const UCHAR &expected2, const UCHAR &received2);
    void wrongTx(const UCHAR &expected, const UCHAR &received);
    void unknownCommand(const UCHAR &command);
};

#endif // MODBUSHANDLER_H
