#include "lightdeviceswindow.h"
#include "ui_lightdeviceswindow.h"
#include <QDebug>

LightDevicesWindow::LightDevicesWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LightDevicesWindow),
    lampList(nullptr),
    lampNode(nullptr)
{
    ui->setupUi(this);

    connect(ui->createLampsCheckBox, &QCheckBox::stateChanged, this, &LightDevicesWindow::onCreateLampsCheckBoxStateChanged);
    connect(ui->lampsCombo, &QComboBox::currentIndexChanged, this, &LightDevicesWindow::onLampsComboIndexChaned);
    connect(ui->devicesCombo, &QComboBox::currentIndexChanged, this, &LightDevicesWindow::onLampsComboIndexChaned);
    connect(ui->saveButton, &QPushButton::clicked, this, &LightDevicesWindow::onSaveButtonClicked);

    ui->statusCombo->addItem("NORMAL");
    ui->statusCombo->addItem("WARNING");
    ui->statusCombo->addItem("ERROR");
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

void LightDevicesWindow::updateLampsComboBox()
{
    ui->lampsCombo->clear();
    for (int i = 1; i <= ui->lampsNumberSpinBox->value(); i++)
    {
        ui->lampsCombo->addItem(QString::number(i));
    }
}

void LightDevicesWindow::updateStatusComboBox(UCHAR status)
{
    if (status == 0x00)
        ui->statusCombo->setCurrentIndex(0);
    else if (status == 0x40)
        ui->statusCombo->setCurrentIndex(1);
    else if (status == 0x80)
        ui->statusCombo->setCurrentIndex(2);
}

void LightDevicesWindow::updateValues()
{
    if (lampNode)
    {
        lampNode->levelHost = ui->hostLevelSpinBox->value();
        lampNode->status = [this]() -> UCHAR {
            switch (ui->statusCombo->currentIndex()) {
            case 0: return 0x00; // STATUS_NORMAL
            case 1: return 0x40; // STATUS_WARNING
            case 2: return 0x80; // STATUS_ERROR
            default: return 0x00;
            }
        }();
    }
}

void LightDevicesWindow::onCreateLampsCheckBoxStateChanged(int state)
{
    if (state)
    {
        ui->lampsNumberSpinBox->setEnabled(false);
        for (auto &device: devices)
        {
            int size = ui->lampsNumberSpinBox->value();
            device->setLampsList(size, 50, 0x80);
        }
        updateLampsComboBox();
    }
    else
        ui->lampsNumberSpinBox->setEnabled(true);
}

void LightDevicesWindow::onLampsComboIndexChaned(int index)
{
    updateValues();
    QString selectedDeviceName = ui->devicesCombo->currentText();
    Device* selectedDevice = devices[selectedDeviceName];

    UINT lampId = ui->lampsCombo->currentIndex() + 1;

    lampList = selectedDevice->getLampList();
    lampNode = lampList->getNodeById(lampId);

    if (lampNode)
    {
        ui->hostLevelSpinBox->setValue(lampNode->levelHost);
        updateStatusComboBox(lampNode->status);
    }
}

void LightDevicesWindow::onSaveButtonClicked()
{
    updateValues();
    for (auto &device : devices)
    {
        lampList = device->getLampList();
        lampList->updateNodes();
        qDebug() << "Updated lamps for " << device;
    }
    this->close();
}
