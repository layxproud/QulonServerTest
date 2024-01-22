#ifndef AHPSTATEWINDOW_H
#define AHPSTATEWINDOW_H

#include "device.h"
#include <QDialog>
#include <QCloseEvent>

namespace Ui {
class AhpStateWindow;
}

class AhpStateWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AhpStateWindow(QWidget *parent = nullptr);
    ~AhpStateWindow();

    void setDevices(const QMap<QString, Device*> &devices);

private:
    Ui::AhpStateWindow *ui;
    USHORT currentValue;
    USHORT prevValue;
    QMap<QString, Device*> devices;

    void changeAhpState();
    QByteArray getStateArray();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onAcceptButtonClicked();
    void onCancelButtonClicked();
};

#endif // AHPSTATEWINDOW_H
