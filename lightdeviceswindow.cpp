#include "lightdeviceswindow.h"
#include "ui_lightdeviceswindow.h"
#include <QDebug>

LightDevicesWindow::LightDevicesWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LightDevicesWindow),
    lampList(nullptr),
    lampNode(nullptr),
    isProgrammaticChange(false)
{
    ui->setupUi(this);

    connect(ui->createLampsCheckBox, &QCheckBox::stateChanged, this, &LightDevicesWindow::onCreateLampsCheckBoxStateChanged);
    connect(ui->lampsCombo, &QComboBox::currentIndexChanged, this, &LightDevicesWindow::onLampsComboIndexChaned);
    connect(ui->devicesCombo, &QComboBox::currentIndexChanged, this, &LightDevicesWindow::onDevicesComboIndexChanged);
    connect(this, &LightDevicesWindow::deviceChanged, this, &LightDevicesWindow::onLampsComboIndexChaned);
    connect(ui->saveButton, &QPushButton::clicked, this, &LightDevicesWindow::onSaveButtonClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &LightDevicesWindow::onCancelButtonClicked);
    connect(ui->setForAllRadio, &QRadioButton::clicked, this, &LightDevicesWindow::onSetForAllToggled);
    connect(ui->setForChosenRadio, &QRadioButton::clicked, this, &LightDevicesWindow::onSetForChosenToggled);
    connect(ui->setRandomRadio, &QRadioButton::clicked, this, &LightDevicesWindow::onSetRandomToggled);

    ui->statusCombo->addItem("NORMAL");
    ui->statusCombo->addItem("WARNING");
    ui->statusCombo->addItem("ERROR");
    ui->statusCombo->setCurrentIndex(0);

    currentState = LampState::SetForChosen;
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

void LightDevicesWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    if (devices.isEmpty())
        return;

    QString selectedDeviceName = ui->devicesCombo->currentText();
    Device* selectedDevice = devices[selectedDeviceName];
    lampList = selectedDevice->getLampList();
    if (lampList->isNodesListEmpty())
        return;

    int size = lampList->getNodesListSize();
    ui->lampsNumberSpinBox->setValue(size);
    isProgrammaticChange = true;
    ui->createLampsCheckBox->setCheckState(Qt::Checked);
    isProgrammaticChange = false;
    updateLampsComboBox();

    UINT lampId = ui->lampsCombo->currentIndex() + 1;
    lampNode = lampList->getNodeById(lampId);
    if (lampNode)
    {
        ui->hostLevelSpinBox->setValue(lampNode->levelHost);
        updateStatusComboBox(lampNode->status);
    }
}

void LightDevicesWindow::updateDeviceComboBox()
{
    disconnect(ui->devicesCombo, &QComboBox::currentIndexChanged, this, &LightDevicesWindow::onDevicesComboIndexChanged);

    ui->devicesCombo->clear();
    switch (currentState)
    {
        case LampState::SetForChosen:
            for (auto &device: devices)
            {
                QString deviceID = device->getPhone();
                ui->devicesCombo->addItem(deviceID);
            }
            ui->devicesCombo->setEnabled(true);
            emit deviceChanged();
            break;

        case LampState::SetForAll:
            ui->devicesCombo->addItem("Все устройства");
            ui->devicesCombo->setEnabled(false);
            break;

        case LampState::SetRandom:
            ui->devicesCombo->addItem("Все устройства");
            ui->devicesCombo->setEnabled(false);
            break;

        default:
            break;
    }

    connect(ui->devicesCombo, &QComboBox::currentIndexChanged, this, &LightDevicesWindow::onDevicesComboIndexChanged);

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

UCHAR LightDevicesWindow::getSelectedStatus()
{
    switch (ui->statusCombo->currentIndex())
    {
        case 0: return 0x00; // STATUS_NORMAL
        case 1: return 0x40; // STATUS_WARNING
        case 2: return 0x80; // STATUS_ERROR
        default: return 0x00;
    }
}

void LightDevicesWindow::updateValues()
{

    if (lampNode)
    {
        lampNode->levelHost = ui->hostLevelSpinBox->value();
        lampNode->status = getSelectedStatus();
    }
}

void LightDevicesWindow::onCreateLampsCheckBoxStateChanged(int state)
{
    if (isProgrammaticChange)
    {
        ui->lampsNumberSpinBox->setEnabled(false);
        return;
    }

    if (state)
    {
        ui->lampsNumberSpinBox->setEnabled(false);
        for (auto &device: devices)
        {
            int size = ui->lampsNumberSpinBox->value();
            device->setLampsList(size, 100, 0x00);
        }
        updateLampsComboBox();
    }
    else
        ui->lampsNumberSpinBox->setEnabled(true);
}

void LightDevicesWindow::onDevicesComboIndexChanged()
{
    if (currentState == LampState::SetForChosen)
        emit deviceChanged();
    return;
}

void LightDevicesWindow::onLampsComboIndexChaned()
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
    }
    this->close();
}

void LightDevicesWindow::onCancelButtonClicked()
{
    for (auto &device : devices)
    {
        lampList = device->getLampList();
        lampList->restoreInitialState();
    }
    QDialog::close();
}

void LightDevicesWindow::onSetForChosenToggled()
{
    currentState = LampState::SetForChosen;
    updateDeviceComboBox();
}

void LightDevicesWindow::onSetForAllToggled()
{
    currentState = LampState::SetForAll;
    updateDeviceComboBox();
}

void LightDevicesWindow::onSetRandomToggled()
{
    currentState = LampState::SetRandom;
    updateDeviceComboBox();
}
