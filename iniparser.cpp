#include "iniparser.h"
#include <QDebug>

IniParser::IniParser(QObject *parent)
    : QObject{parent}
{
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
        qDebug() << "Failed to open .ini file" << file.errorString();
        return;
    }

    QTextStream in(&file);

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
                // Проверяем, существует ли уже устройство с таким "Phone"
                if (!devices.contains(setDevice["phone"]))
                {
                    Device* device = new Device(setDevice["phone"], setDevice["name"]);
                    devices.insert(setDevice["phone"], device);
                }
                else
                {
                    qDebug() << "Устройство с номером" << setDevice["phone"] << "уже существует.";
                }
            }
        }
    }

    file.close();
}
