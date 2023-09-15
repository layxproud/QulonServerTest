#ifndef INIPARSER_H
#define INIPARSER_H

#include <QObject>

#include "device.h"
#include "logger.h"
#include <QFile>
#include <QTextStream>

class IniParser : public QObject
{
    Q_OBJECT
public:
    explicit IniParser(Logger *logger, QObject *parent = nullptr);

    void parseIniFile(const QString &filePath);

public:
    QMap<QString, QString> gprsSettings;
    QMap<QString, Device*> devices;

private:
    // Logger
    Logger *loggerInstance;

private:
    QMap<QString, QString> parseSection(QTextStream& in, const QStringList& keys);

    void setLogger(Logger *logger);

};

#endif // INIPARSER_H
