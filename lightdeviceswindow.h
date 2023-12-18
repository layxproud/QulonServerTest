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
    LampList* lampList;
    Node* lampNode;

    void updateDeviceComboBox();
    void updateLampsComboBox();
    void updateStatusComboBox(UCHAR status);
    void updateValues();

private slots:
    void onCreateLampsCheckBoxStateChanged(int state);
    void onLampsComboIndexChaned(int index);
    void onSaveButtonClicked();

};

#endif // LIGHTDEVICESWINDOW_H
