#ifndef LIGHTDEVICESWINDOW_H
#define LIGHTDEVICESWINDOW_H

#include <QDialog>
#include "device.h"

namespace Ui {
class LightDevicesWindow;
}

enum class LampState {
    SetForAll,
    SetForChosen,
    SetRandom
};

class LightDevicesWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LightDevicesWindow(QWidget *parent = nullptr);
    ~LightDevicesWindow();

    void setDevices(const QMap<QString, Device*> &devices);

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::LightDevicesWindow *ui;
    QMap<QString, Device*> devices;
    LampList* lampList;
    Node* lampNode;
    LampState currentState;
    UINT prevLampID;
    UINT currLampID;

    bool isProgrammaticChange;

private:
    void updateDeviceComboBox();
    void updateLampsComboBox();
    void updateStatusComboBox(UCHAR status);
    UCHAR getSelectedStatus();
    void updateValues();

signals:
    void deviceChanged();

private slots:
    void onCreateLampsCheckBoxStateChanged(int state);
    void onDevicesComboIndexChanged();
    void onLampsComboIndexChaned();
    void onSaveButtonClicked();
    void onCancelButtonClicked();
    void onSetForChosenToggled();
    void onSetForAllToggled();
    void onSetRandomToggled();

};

#endif // LIGHTDEVICESWINDOW_H
