#include "iniparser.h"
#include <QDebug>

IniParser::IniParser(Logger *logger, QObject *parent)
    : QObject{parent}
{
    setLogger(logger);
}

void IniParser::setLogger(Logger *logger)
{
    loggerInstance = logger;
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
        loggerInstance->logError(tr("Не удалось открыть .ini файл: ") + file.errorString());
        return;
    }

    QTextStream in(&file);
    in.setCodec("Windows-1251");

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
                    Device* device = new Device(setDevice["phone"], setDevice["name"], loggerInstance);
                    devices.insert(setDevice["phone"], device);
                }
                else
                {
                    loggerInstance->logInfo(tr("Устройство с номером ") + setDevice["phone"] + tr(" уже существует"));
                }
            }
        }
    }

    file.close();
}
