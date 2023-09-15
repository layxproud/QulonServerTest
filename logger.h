#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QTextBrowser>

class Logger : public QObject
{
    Q_OBJECT
public:
    Logger(QObject *parent = nullptr);

    void setLogWindow(QTextBrowser *logWindow);

    void logInfo(const QString &message);
    void logWarning(const QString &message);
    void logError(const QString &message);

    QString byteArrToStr(const QByteArray &arr);

private:
    QTextBrowser *logWindow;
};

#endif // LOGGER_H
