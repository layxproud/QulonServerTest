#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    logger = new Logger(this);
    logger->setLogWindow(ui->logWindow);
    iniParser = new IniParser(logger, this);

    ui->connectIntervalBox->setValue(1);
    ui->disconnectIntervalBox->setValue(20);
}

MainWindow::~MainWindow()
{
    cleanupDevices();
    iniParser->clearData();
    delete ui;
}


void MainWindow::populateDeviceTable(const QMap<QString, Device*> &devices)
{
    ui->tableWidget->clear();

    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setRowCount(devices.size());

    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("ID")
                                                             << tr("Имя")
                                                             << tr("Статус соединения"));

    ui->tableWidget->setAlternatingRowColors(true);

    // Size Policy
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    int row = 0;
    foreach (const QString &phone, devices.keys())
    {
        Device* device = devices.value(phone);
        device->setIp(iniParser->gprsSettings["ip"]);
        device->setPort(iniParser->getPort());

        QTableWidgetItem* phoneItem = new QTableWidgetItem(device->getPhone());
        QTableWidgetItem* nameItem = new QTableWidgetItem(device->getName());
        QTableWidgetItem* statusItem = new QTableWidgetItem();

        connect(device, &Device::connected, this, [=]() {
            statusItem->setText(tr("Подключено"));
        });

        connect(device, &Device::disconnected, this, [=]() {
            statusItem->setText(tr("Нет соединения"));
        });

        if (device->_client.isConnected())
        {
            statusItem->setText(tr("Подключено"));
        }
        else
        {
            statusItem->setText(tr("Нет соединения"));
        }

        ui->tableWidget->setItem(row, 0, phoneItem);
        ui->tableWidget->setItem(row, 1, nameItem);
        ui->tableWidget->setItem(row, 2, statusItem);

        deviceList.append(device);
        ++row;
    }
}

void MainWindow::cleanupDevices()
{
    for (Device* device : deviceList)
    {
        delete device;
    }
    deviceList.clear();

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
}


void MainWindow::on_openIniFileAction_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Выберите ini файл"),
                                                    QDir::homePath(),
                                                    tr("Config files (*.ini)"));
    if (!filePath.isEmpty())
    {
        cleanupDevices();
        iniParser->clearData();
        iniParser->parseIniFile(filePath);
        populateDeviceTable(iniParser->devices);
        ui->ipValue->setText(iniParser->gprsSettings["ip"]);
        ui->portValue->setText(iniParser->gprsSettings["port"]);
    }
    else
    {
        logger->logError(tr("Ошибка открытия файла. Файл пустой или не существует."));
    }
}


void MainWindow::on_connectButton_clicked()
{
    bool ok;

    QString phoneNumber = QInputDialog::getText(this,
                                                tr("Введите номер телефона"),
                                                tr("Номер телефона:"),
                                                QLineEdit::Normal,
                                                QString(),
                                                &ok);

    if (ok && !phoneNumber.isEmpty())
    {
        if (iniParser->devices.contains(phoneNumber))
        {
            Device* device = iniParser->devices.value(phoneNumber);
            device->_client.connectToServer(iniParser->gprsSettings["ip"], iniParser->getPort());
        }
        else
        {
            logger->logWarning(tr("Устройство с номером ") + phoneNumber + tr(" не найдено в списке."));
        }
    }
}


void MainWindow::on_pushButton_clicked()
{
    foreach (Device* device, iniParser->devices)
    {
        device->startConnectionTimer();
    }
}


void MainWindow::on_connectIntervalBox_valueChanged(int arg1)
{
    foreach (Device* device, iniParser->devices)
    {
        device->setConnectionInterval(arg1);
    }
}


void MainWindow::on_disconnectIntervalBox_valueChanged(int arg1)
{
    foreach (Device* device, iniParser->devices)
    {
        device->setDisconnectionInterval(arg1);
    }
}

