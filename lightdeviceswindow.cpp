#include "lightdeviceswindow.h"
#include "ui_lightdeviceswindow.h"
#include <QDebug>

LightDevicesWindow::LightDevicesWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LightDevicesWindow)
{
    ui->setupUi(this);

    connect(ui->createLampsCheckBox, &QCheckBox::stateChanged, this, &LightDevicesWindow::onCreateLampsCheckBoxStateChanged);
}

void LightDevicesWindow::setDevices(const QMap<QString, Device *> &devices)
{
    this->devices = devices;
    updateDeviceComboBox();
}


LightDevicesWindow::~LightDevicesWindow()
{
    delete ui;
}

void LightDevicesWindow::updateDeviceComboBox()
{
    ui->devicesCombo->clear();
    for (auto &device: devices)
    {
        QString deviceID = device->getPhone();
        ui->devicesCombo->addItem(deviceID);
    }
}

void LightDevicesWindow::onCreateLampsCheckBoxStateChanged(int state)
{
    if (state)
        ui->lampsNumberSpinBox->setEnabled(false);
    else
        ui->lampsNumberSpinBox->setEnabled(true);
}
