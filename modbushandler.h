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
    void formStateMessage(const bool &outsideCall);
    void randomiseRelayStates();
    void addFileToMap(const QString &fileName, const QByteArray &fileData);
    void editState(const UCHAR &stateByte, const QByteArray &data);

private:
    const QByteArray SYNC_MESSAGE = QByteArray::fromHex("00800010");

    QString devicePhone;
    UCHAR currentTx;
    UCHAR currentRx;
    UCHAR deviceAddress;
    UCHAR serverAddress;

    QByteArray currentMessage;
    QByteArray receivedMessage;

    // Stores relay state. Has default states, new can be added
    std::vector<FL_MODBUS_STATE_CMD_MESSAGE> stateMessage;
    // Map for storing all virtual files.
    QMap<QString, QByteArray> filesMap;
    // Map's iterator to store current file.
    QMap<QString, QByteArray>::iterator currentFileIterator;
    // Stores current file
    QByteArray currentFileInfo;
    QByteArray currentFileData;
    bool endOfFile;

    void performCommand(const QByteArray &message);
    void formSyncMessage();
    void setRelay(const QByteArray &message);
    void formDefaultAnswer(const QByteArray &message);
    void formIdentificationMessage();
    void addNewState(const UCHAR &type, const QByteArray &data);
    void initFileSearch(const QByteArray &message);
    void searchFile(const QByteArray &message);
    void fileResult(bool calledAsResult);
    void openReadFile(const QByteArray &message);
    void readFile(const QByteArray &message);
    void closeFile(const QByteArray &message);
    void replyError(UCHAR errorCode);

    void editRelayByte(UCHAR relayByte);
    void editRelayByte(const QByteArray &relayMask);

    // 0x2C block
    uint8_t counterArray[88] = {0};
    int current = 0;
    int power = 0;
    int voltage = 220;
    double frequency = 50;
    uint32_t nullValue = 4294967295;
    void editCounterArrayByte(uint8_t* buffer, int serialNumber, uint32_t value);
    void calcNewCurrent();
    void calcNewPower();
    void calcFrequency();
    void editCounterArray();

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

public slots:
    void parseMessage(const QByteArray &rawMessage);
};

#endif // MODBUSHANDLER_H
