#include "logger.h"

Logger::Logger(QObject *parent)
    : QObject{parent}
    , logWindow(nullptr)
    , closing(false)
{}

void Logger::setLogWindow(QTextBrowser *logBrowser)
{
    logWindow = logBrowser;
}

void Logger::disableGUI()
{
    closing = true;
}

void Logger::logInfo(const QString &message)
{
    if (closing) return;

    if (logWindow)
    {
        logWindow->append(QString("<font color='green'>[INFO] %1</font>").arg(message));
    }
}

void Logger::logWarning(const QString &message)
{
    if (closing) return;

    if (logWindow)
    {
        logWindow->append(QString("<font color='orange'>[WARNING] %1</font>").arg(message));
    }
}

void Logger::logError(const QString &message)
{
    if (closing) return;

    if (logWindow)
    {
        logWindow->append(QString("<font color='red'>[ERROR] %1</font>").arg(message));
    }
}

QString Logger::byteArrToStr(const QByteArray &arr)
{
    QString result;
    for (const char &byte : arr)
    {
        result.append(QString("%1 ").arg(static_cast<unsigned char>(byte), 2, 16, QChar('0')));
    }
    return result.trimmed();
}
