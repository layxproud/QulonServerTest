#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , logger(new Logger(this))
    , iniParser(new IniParser(logger, this))
    , isRunning(false)
{
    ui->setupUi(this);
    logger->setLogWindow(ui->logWindow);

    spinBoxes[0] = {ui->conIntMinBox, DEFAULT_CONNECT_MIN_BOX};
    spinBoxes[1] = {ui->conIntSecBox, DEFAULT_CONNECT_SEC_BOX};
    spinBoxes[2] = {ui->discIntFromMinBox, DEFAULT_DISCONNECT_FROM_MIN_BOX};
    spinBoxes[3] = {ui->discIntFromSecBox, DEFAULT_DISCONNECT_FROM_SEC_BOX};
    spinBoxes[4] = {ui->discIntToMinBox, DEFAULT_DISCONNECT_TO_MIN_BOX};
    spinBoxes[5] = {ui->discIntToSecBox, DEFAULT_DISCONNECT_TO_SEC_BOX};
    spinBoxes[6] = {ui->statusIntMinBox, DEFAULT_STATUS_MIN_BOX};
    spinBoxes[7] = {ui->statusIntSecBox, DEFAULT_STATUS_SEC_BOX};
    initSpinBoxes();
    enableSpinBoxes(true);

    // GUI connects
    connect(ui->multiConnectButton, &QPushButton::clicked, this, &MainWindow::onMultiConnectButtonClicked);
    connect(ui->connectButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(ui->saveValuesButton, &QCheckBox::stateChanged, this, &MainWindow::onSaveValuesButtonStateChanged);
    connect(ui->openIniFileAction, &QAction::triggered, this, &MainWindow::onOpenIniFileActionTriggered);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::populateDeviceTable(const QMap<QString, Device*> &devices)
{
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setRowCount(devices.size());

    // Set table headers
    QStringList headers;
    headers << tr("ID") << tr("Имя") << tr("Статус соединения");
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    ui->tableWidget->setAlternatingRowColors(true);

    // Size Policy
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    int row = 0;
    for (const auto& device : devices)
    {
        QTableWidgetItem* phoneItem = new QTableWidgetItem(device->getPhone());
        QTableWidgetItem* nameItem = new QTableWidgetItem(device->getName());
        QTableWidgetItem* statusItem = new QTableWidgetItem(device->isConnected() ? tr("Подключено") : tr("Нет соединения"));

        connect(device, &Device::connectionChanged, this, [=](bool status) {
            updateDeviceStatus(statusItem, status ? tr("Подключено") : tr("Нет соединения"));
        });

        ui->tableWidget->setItem(row, 0, phoneItem);
        ui->tableWidget->setItem(row, 1, nameItem);
        ui->tableWidget->setItem(row, 2, statusItem);

        ++row;
    }
}

void MainWindow::updateIntervals()
{
    int connectionInterval = ui->conIntMinBox->value() * 60 * 1000 + ui->conIntSecBox->value() * 1000;
    int disconnectionFromInterval = ui->discIntFromMinBox->value() * 60 * 1000 + ui->discIntFromSecBox->value() * 1000;
    int disconnectionToInterval = ui->discIntToMinBox->value() * 60 * 1000 + ui->discIntToSecBox->value() * 1000;
    int sendStatusInterval = ui->statusIntMinBox->value() * 60 * 1000 + ui->statusIntSecBox->value() * 1000;
    for(const auto& device : iniParser->devices)
    {
        device->setConnectionInterval(connectionInterval);
        device->setDisconnectionInterval(disconnectionFromInterval, disconnectionToInterval);
        device->setSendStatusInterval(sendStatusInterval);
    }
}

void MainWindow::initSpinBoxes()
{
    for (const SpinBoxInfo& info : spinBoxes)
    {
        info.spinBox->setValue(info.defaultValue);
    }
}

void MainWindow::enableSpinBoxes(const bool &arg)
{
    for (const SpinBoxInfo& info : spinBoxes)
    {
        info.spinBox->setEnabled(arg);
    }
}
\
void MainWindow::onConnectButtonClicked()
{
    if (ui->tableWidget->rowCount() <= 0)
    {
        logger->logWarning(tr("Устройства не найдены!"));
        return;
    }

    bool ok;

    QString phoneNumber = QInputDialog::getText(this,
                                                tr("Введите номер телефона"),
                                                tr("Номер телефона:"),
                                                QLineEdit::Normal,
                                                QString(),
                                                &ok);

    if (!ok || phoneNumber.isEmpty())
    {
        logger->logError(tr("Ошибка ввода номера телефона."));
        return;
    }

    if (!iniParser->devices.contains(phoneNumber))
    {
        logger->logWarning(tr("Устройство с номером ") + phoneNumber + tr(" не найдено в списке."));
        return;
    }

    Device* device = iniParser->devices.value(phoneNumber);
    device->debugConnect(iniParser->gprsSettings["ip"], iniParser->getPort());
}

void MainWindow::onMultiConnectButtonClicked()
{
    if (ui->tableWidget->rowCount() <= 0)
    {
        logger->logWarning(tr("Устройства не найдены!"));
        return;
    }
    if (!isRunning)
    {
        updateIntervals();
        ui->saveValuesButton->setChecked(true);
        logger->logInfo(tr("Запускаю соединения..."));
        for(const auto& device : iniParser->devices)
        {
            device->startWork();
        }
        ui->multiConnectButton->setText(tr("СТОП"));
        isRunning = true;
    }
    else
    {
        logger->logInfo(tr("Закрываю соединения..."));
        for(const auto& device : iniParser->devices)
        {
            device->stopWork();
        }
        ui->multiConnectButton->setText(tr("СТАРТ"));
        logger->logInfo(tr("Завершено!"));
        isRunning = false;
    }
}

void MainWindow::onOpenIniFileActionTriggered()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Выберите ini файл"),
                                                    QDir::homePath(),
                                                    tr("Config files (*.ini)"));

    if (filePath.isEmpty())
    {
        logger->logError(tr("Ошибка открытия файла."));
        return;
    }

    if (!QFile::exists(filePath))
    {
        logger->logError(tr("Файл не существует."));
        return;
    }

    iniParser->clearData();
    iniParser->parseIniFile(filePath);
    populateDeviceTable(iniParser->devices);
    ui->ipValue->setText(iniParser->gprsSettings["ip"]);
    ui->portValue->setText(iniParser->gprsSettings["port"]);
}

void MainWindow::onSaveValuesButtonStateChanged(int state)
{
    if (state == Qt::Checked)
    {
        enableSpinBoxes(false);
    }
    else if (state == Qt::Unchecked)
    {
        enableSpinBoxes(true);
    }
}

void MainWindow::updateDeviceStatus(QTableWidgetItem *item, const QString &status)
{
    item->setText(status);
}
