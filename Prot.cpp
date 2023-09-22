#include "Prot.h"

void CalculateCRC(const FL_MODBUS_MESSAGE &mm, const QByteArray &data, UCHAR crc[2])
{
    UCHAR c, CRChi, CRClo;
    CRChi = CRClo = 0xFF;

    if(mm.FUNCT == 0x6E)
    {
        char* bf = (char*)&mm;
        for(unsigned int i = 0; i < sizeof(FL_MODBUS_MESSAGE); i++)
        {
            c = bf[i];
            c ^= CRClo;
            CRClo = CRChi^CRC_Table_Hi[c];
            CRChi = CRC_Table_Lo[c];
        };
    }
    else
    {
        char* bf = (char*)&mm.message_short;
        for(unsigned int i=0; i < sizeof(FL_MODBUS_MESSAGE_SHORT); i++)
        {
            c = bf[i];
            c ^= CRClo;
            CRClo = CRChi ^ CRC_Table_Hi[c];
            CRChi = CRC_Table_Lo[c];
        };
    }

    for(unsigned int i = 0; i < data.size(); i++)
    {
        c = data[i];
        c ^= CRClo;
        CRClo = CRChi ^ CRC_Table_Hi[c];
        CRChi = CRC_Table_Lo[c];
    };

    crc[0] = CRClo;
    crc[1] = CRChi;
}

void CalculateCRC(QByteArray &msg)
{
    UCHAR c, CRChi, CRClo;
    CRChi = CRClo = 0xFF;

    for (unsigned int i = 0; i < msg.size(); i++)
    {
        c = msg[i];
        c ^= CRClo;
        CRClo = CRChi ^ CRC_Table_Hi[c];
        CRChi = CRC_Table_Lo[c];
    };

    msg.append(CRClo);
    msg.append(CRChi);
}
