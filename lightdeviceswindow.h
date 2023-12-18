#ifndef LIGHTDEVICESWINDOW_H
#define LIGHTDEVICESWINDOW_H

#include <QDialog>
#include "device.h"

namespace Ui {
class LightDevicesWindow;
}

class LightDevicesWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LightDevicesWindow(QWidget *parent = nullptr);
    void setDevices(const QMap<QString, Device*> &devices);

    ~LightDevicesWindow();

private:
    Ui::LightDevicesWindow *ui;
    QMap<QString, Device*> devices;

    void updateDeviceComboBox();

private slots:
    void onCreateLampsCheckBoxStateChanged(int state);


};

#endif // LIGHTDEVICESWINDOW_H
