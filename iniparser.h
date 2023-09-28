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
    quint16 getPort();

    void clearData();

public:
    QMap<QString, QString> gprsSettings;
    QMap<QString, Device*> devices;

private:
    Logger *loggerInstance;

private:
    void setLogger(Logger *logger);
    QMap<QString, QString> parseSection(QTextStream& in, const QStringList& keys);
};

#endif // INIPARSER_H
