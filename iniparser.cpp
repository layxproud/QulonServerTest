#include "iniparser.h"
#include <QDebug>
#include <QTextCodec>

IniParser::IniParser(Logger *logger, QObject *parent)
    : QObject{parent}
    , _logger{logger}
{

}

IniParser::~IniParser()
{
    clearData();
}

QMap<QString, QString> IniParser::parseSection(QTextStream& in, const QStringList& keys)
{
    QMap<QString, QString> sectionData;

    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.startsWith("}"))
        {
            break;
        }

        for (const QString& key : keys)
        {
            if (line.startsWith(key + "="))
            {
                QString value = line.mid(key.length() + 2, line.length() - key.length() - 3);
                sectionData.insert(key, value);
                break;
            }
        }
    }

    return sectionData;
}

void IniParser::parseIniFile(const QString& filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        _logger->logError(tr("Не удалось открыть .ini файл: ") + file.errorString());
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::System);
    // не работает если в системе выставить "Использовать Юникод (UTF-8) для поддержки языка во всем мире"
//    QByteArray fileContent = file.readAll();
//    QTextCodec* codec1251 = QTextCodec::codecForName("Windows-1251");
//    QString decodedContent = codec1251->toUnicode(fileContent);
//    QTextStream in(&decodedContent, QIODevice::ReadOnly);

    QString currentSection;

    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();

        if (line.startsWith('#'))
        {
            QString section = line.mid(1);
            currentSection = section;
        }
        else if (line.startsWith("{"))
        {
            if (currentSection == "GPRSSETTINGS")
            {
                QStringList keys = { "ip", "port" };
                gprsSettings = parseSection(in, keys);
            }
            else if (currentSection == "SETDEVICE")
            {
                QStringList keys = { "phone", "name" };
                QMap<QString, QString> setDevice = parseSection(in, keys);
                if (!devices.contains(setDevice["phone"]))
                {
                    Device* device = new Device(setDevice["phone"], setDevice["name"], _logger);
                    devices.insert(setDevice["phone"], device);

                    device->setIp(gprsSettings["ip"]);
                    device->setPort(getPort());
                }
                else
                {
                    _logger->logWarning(tr("Устройство с номером ") + setDevice["phone"] + tr(" уже существует"));
                }
            }
        }
    }

    file.close();
}

quint16 IniParser::getPort()
{
    bool ok;
    QString portStr = gprsSettings["port"];
    quint16 port = portStr.toUInt(&ok);
    return port;
}

void IniParser::clearData()
{
    for (auto &device : devices)
    {
        delete device;
    }

    devices.clear();
    gprsSettings.clear();
}
