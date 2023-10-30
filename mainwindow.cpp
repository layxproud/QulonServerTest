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
    spinBoxes[6] = {ui->sendStatusIntMinBox, DEFAULT_SEND_STATUS_MIN_BOX};
    spinBoxes[7] = {ui->sendStatusIntSecBox, DEFAULT_SEND_STATUS_SEC_BOX};
    spinBoxes[8] = {ui->changeStatusIntMinBox, DEFAULT_CHANGE_STATUS_MIN_BOX};
    spinBoxes[9] = {ui->changeStatusIntSecBox, DEFAULT_CHANGE_STATUS_SEC_BOX};
    initSpinBoxes();
    enableSpinBoxes(true);

    // GUI connects
    connect(ui->multiConnectButton, &QPushButton::clicked, this, &MainWindow::onMultiConnectButtonClicked);
    connect(ui->connectButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(ui->saveValuesButton, &QCheckBox::stateChanged, this, &MainWindow::onSaveValuesButtonStateChanged);
    connect(ui->openIniFileAction, &QAction::triggered, this, &MainWindow::onOpenIniFileActionTriggered);
    connect(ui->relayManualButton, &QRadioButton::toggled, this, &MainWindow::onRelayManualButtonToggled);
    connect(ui->sendRelayBitsButton, &QPushButton::clicked, this, &MainWindow::onSendRelayBitsButtonClicked);

    // Status bar
    ui->statusBar->addWidget(ui->ipLabel);
    ui->statusBar->addWidget(ui->ipValue);
    ui->statusBar->addWidget(ui->portLabel);
    ui->statusBar->addWidget(ui->portValue);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (logger)
        logger->disableGUI();

    QMainWindow::closeEvent(event);
}

void MainWindow::populateDeviceTable(const QMap<QString, Device*> &devices)
{
    ui->tableWidget->clear();


    // Set table headers
    QStringList headers;
    headers << tr("") << tr("ID") << tr("Имя") << tr("Статус соединения");
    ui->tableWidget->setColumnCount(headers.size());
    ui->tableWidget->setRowCount(devices.size());
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    ui->tableWidget->setAlternatingRowColors(true);

    // Size Policy
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    QMap<int, QCheckBox*> checkboxMap;

    int row = 0;
    for (const auto& device : devices)
    {
        QCheckBox* checkBox = new QCheckBox();
        ui->tableWidget->setCellWidget(row, 0, checkBox);
        checkboxMap[row] = checkBox;

        QTableWidgetItem* phoneItem = new QTableWidgetItem(device->getPhone());
        QTableWidgetItem* nameItem = new QTableWidgetItem(device->getName());
        QTableWidgetItem* statusItem = new QTableWidgetItem(device->isConnected() ? tr("Подключено") : tr("Нет соединения"));

        connect(device, &Device::connectionChanged, this, [=](bool status) {
            updateDeviceStatus(statusItem, status ? tr("Подключено") : tr("Нет соединения"));
        });

        ui->tableWidget->setItem(row, 1, phoneItem);
        ui->tableWidget->setItem(row, 2, nameItem);
        ui->tableWidget->setItem(row, 3, statusItem);

        ++row;
    }
}

void MainWindow::updateDeviceDefaults()
{
    int connectionInterval = ui->conIntMinBox->value() * 60 * 1000 + ui->conIntSecBox->value() * 1000;
    int disconnectionFromInterval = ui->discIntFromMinBox->value() * 60 * 1000 + ui->discIntFromSecBox->value() * 1000;
    int disconnectionToInterval = ui->discIntToMinBox->value() * 60 * 1000 + ui->discIntToSecBox->value() * 1000;
    int sendStatusInterval = ui->sendStatusIntMinBox->value() * 60 * 1000 + ui->sendStatusIntSecBox->value() * 1000;
    int changeStatusInterval = ui->changeStatusIntMinBox->value() * 60 * 1000 + ui->changeStatusIntSecBox->value() * 1000;
    for(const auto& device : iniParser->devices)
    {
        device->setConnectionInterval(connectionInterval);
        device->setDisconnectionInterval(disconnectionFromInterval, disconnectionToInterval);
        device->setSendStatusInterval(sendStatusInterval);
        device->setChangeStatusInterval(changeStatusInterval);
        device->setAutoRegen(ui->relayAutoButton->isChecked());
    }
    editByteForSelected(calculateByte());
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

UCHAR MainWindow::calculateByte()
{
    UCHAR resultByte = 0;

    // Получаем состояния чекбоксов и устанавливаем соответствующие биты в байте
    if (ui->bit0->isChecked()) resultByte |= 0x01;
    if (ui->bit1->isChecked()) resultByte |= 0x02;
    if (ui->bit2->isChecked()) resultByte |= 0x04;
    if (ui->bit3->isChecked()) resultByte |= 0x08;
    if (ui->bit4->isChecked()) resultByte |= 0x10;
    if (ui->bit5->isChecked()) resultByte |= 0x20;
    if (ui->bit6->isChecked()) resultByte |= 0x40;
    if (ui->bit7->isChecked()) resultByte |= 0x80;

    return resultByte;
}

void MainWindow::editByteForSelected(const UCHAR& byte)
{
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
    {
        QCheckBox* checkBox = qobject_cast<QCheckBox*>(ui->tableWidget->cellWidget(row, 0));
        if (checkBox && checkBox->isChecked())
        {
            // This checkbox is selected, perform action on the corresponding Device object
            Device* device = iniParser->devices.value(ui->tableWidget->item(row, 1)->text());

            if (device)
            {
                qDebug () << "In " << device->getPhone() << " changing bytes";
                device->editByte(byte);
            }
        }
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

    if (!ok) return;

    if (phoneNumber.isEmpty())
    {
        logger->logWarning(tr("Вы не ввели номер!"));
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
        updateDeviceDefaults();
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

void MainWindow::onRelayManualButtonToggled(bool checked)
{
    ui->bit0->setEnabled(checked);
    ui->bit1->setEnabled(checked);
    ui->bit2->setEnabled(checked);
    ui->bit3->setEnabled(checked);
    ui->bit4->setEnabled(checked);
    ui->bit5->setEnabled(checked);
    ui->bit6->setEnabled(checked);
    ui->bit7->setEnabled(checked);
    ui->sendRelayBitsButton->setEnabled(checked);

    // disable auto regen in devices
    for(auto& device : iniParser->devices)
    {
        device->setAutoRegen(!checked);
    }
}

void MainWindow::onSendRelayBitsButtonClicked()
{
    UCHAR resultByte = calculateByte();

    editByteForSelected(resultByte);

    qDebug() << "Result Byte: " << QString::number(resultByte, 16);
}


