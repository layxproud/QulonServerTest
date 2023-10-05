#ifndef INIPARSER_H
#define INIPARSER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include "device.h"
#include "logger.h"

class IniParser : public QObject
{
    Q_OBJECT
public:
    explicit IniParser(Logger *logger, QObject *parent = nullptr);
    ~IniParser();

    void parseIniFile(const QString &filePath);
    quint16 getPort();

    void clearData();

public:
    QMap<QString, QString> gprsSettings;
    QMap<QString, Device*> devices;

private:
    Logger *_logger;

private:
    QMap<QString, QString> parseSection(QTextStream& in, const QStringList& keys);
};

#endif // INIPARSER_H
