#include "ahpstatewindow.h"
#include "ui_ahpstatewindow.h"
#include <QDataStream>

AhpStateWindow::AhpStateWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AhpStateWindow),
    currentValue{0},
    prevValue{0},
    devices{}
{
    ui->setupUi(this);
    setFixedSize(200, 100);

    connect(ui->acceptButton, &QPushButton::clicked, this, &AhpStateWindow::onAcceptButtonClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &AhpStateWindow::onCancelButtonClicked);
}

AhpStateWindow::~AhpStateWindow()
{
    delete ui;
}

void AhpStateWindow::setDevices(const QMap<QString, Device *> &devices)
{
    this->devices = devices;
}

void AhpStateWindow::changeAhpState()
{
    for (const auto &device : devices)
    {
        device->editAhpState(getStateArray());
    }
}

QByteArray AhpStateWindow::getStateArray()
{
    QByteArray ahpState;
    ahpState.append(static_cast<char>(sizeof(currentValue)));

    if (ui->randomButton->isChecked())
        currentValue = QRandomGenerator::global()->bounded(currentValue + 1);

    QByteArray temp;

    QDataStream stream(&temp, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << currentValue;

    ahpState.append(temp);

    return ahpState;
}

void AhpStateWindow::closeEvent(QCloseEvent *event)
{
    ui->ahpValueSpinBox->setValue(prevValue);
    hide();
    event->ignore();
}

void AhpStateWindow::onAcceptButtonClicked()
{
    currentValue = ui->ahpValueSpinBox->value();
    prevValue = currentValue;
    changeAhpState();
    hide();
}

void AhpStateWindow::onCancelButtonClicked()
{
    ui->ahpValueSpinBox->setValue(prevValue);
    hide();
}
