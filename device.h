#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>

class Device : public QObject
{
    Q_OBJECT
public:
    explicit Device(QObject *parent = nullptr);
    Device(const QString &phone, const QString &name, QObject *parent = nullptr);

    QString getPhone() const;
    QString getName() const;
    bool isConnected() const;

private:
    QString _phone;
    QString _name;
    bool _connected = false;
};

#endif // DEVICE_H
