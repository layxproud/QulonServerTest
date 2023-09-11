#ifndef INIPARSER_H
#define INIPARSER_H

#include <QObject>

#include "device.h"
#include <QFile>
#include <QTextStream>

class IniParser : public QObject
{
    Q_OBJECT
public:
    explicit IniParser(QObject *parent = nullptr);

    void parseIniFile(const QString &filePath);

public:
    QMap<QString, QString> gprsSettings;
    QMap<QString, Device*> devices;

private:
    QMap<QString, QString> parseSection(QTextStream& in, const QStringList& keys);

};

#endif // INIPARSER_H
