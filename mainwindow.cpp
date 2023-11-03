#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "checkboxheader.h"
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

    initStatusBar();

    // GUI connects
    connect(ui->multiConnectButton, &QPushButton::clicked, this, &MainWindow::onMultiConnectButtonClicked);
    connect(ui->connectButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(ui->saveValuesButton, &QCheckBox::stateChanged, this, &MainWindow::onSaveValuesButtonStateChanged);
    connect(ui->openIniFileAction, &QAction::triggered, this, &MainWindow::onOpenIniFileActionTriggered);
    connect(ui->relayManualButton, &QRadioButton::toggled, this, &MainWindow::onRelayManualButtonToggled);
    connect(ui->sendState21BitsButton, &QPushButton::clicked, this, &MainWindow::onSendState21BitsButtonClicked);
    connect(ui->sendState23BitsButton, &QPushButton::clicked, this, &MainWindow::onSendState23BitsButtonClicked);
    connect(ui->tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onSelectionChanged);
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

void MainWindow::initStatusBar()
{
    ipLabel = new QLabel("IP:");
    ipValue = new QLabel("");
    portLabel = new QLabel("Port:");
    portValue = new QLabel("");

    ui->statusBar->addWidget(ipLabel);
    ui->statusBar->addWidget(ipValue);
    ui->statusBar->addWidget(portLabel);
    ui->statusBar->addWidget(portValue);
}

void MainWindow::populateDeviceTable(const QMap<QString, Device*> &devices)
{
    ui->tableWidget->clear();
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    QStringList headers;
    headers << tr("") << tr("ID") << tr("Имя") << tr("Статус соединения");
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    // Height Width
    ui->tableWidget->setColumnCount(headers.size());
    ui->tableWidget->setRowCount(devices.size());
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

    // Header
    QCheckBox *headerCheckBox = new QCheckBox(ui->tableWidget->horizontalHeader());
    int firstColumnWidth = ui->tableWidget->columnWidth(0);
    int headerHeight = ui->tableWidget->horizontalHeader()->height();
    headerCheckBox->setGeometry(0, 0, firstColumnWidth, headerHeight);

    connect(ui->tableWidget->horizontalHeader(), &QHeaderView::sectionResized, this, [=](int logicalIndex, int oldSize, int newSize) {
        if (logicalIndex == 0)
        {
            headerCheckBox->setGeometry(0, 0, newSize, headerHeight);
        }
    });
}

void MainWindow::updateDeviceDefaults()
{
    int connectionInterval = ui->conIntMinBox->value() * SEC * MILSEC + ui->conIntSecBox->value() * MILSEC;
    int disconnectionFromInterval = ui->discIntFromMinBox->value() * SEC * MILSEC + ui->discIntFromSecBox->value() * MILSEC;
    int disconnectionToInterval = ui->discIntToMinBox->value() * SEC * MILSEC + ui->discIntToSecBox->value() * MILSEC;
    int sendStatusInterval = ui->sendStatusIntMinBox->value() * SEC * MILSEC + ui->sendStatusIntSecBox->value() * MILSEC;
    int changeStatusInterval = ui->changeStatusIntMinBox->value() * SEC * MILSEC + ui->changeStatusIntSecBox->value() * MILSEC;
    for(const auto& device : iniParser->devices)
    {
        device->setConnectionInterval(connectionInterval);
        device->setDisconnectionInterval(disconnectionFromInterval, disconnectionToInterval);
        device->setSendStatusInterval(sendStatusInterval);
        device->setChangeStatusInterval(changeStatusInterval);
        device->setAutoRegen(ui->relayAutoButton->isChecked());
    }
    // editByteForSelected(getCurrentTab(), calculateByte());
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

QByteArray MainWindow::calculateByte()
{
    QByteArray calculatedByte{};
    UCHAR resultByte{};
    switch (ui->relayStates->currentIndex())
    {
        case 0:
            if (ui->state21_bit0->isChecked()) resultByte |= 0x01;
            if (ui->state21_bit1->isChecked()) resultByte |= 0x02;
            if (ui->state21_bit2->isChecked()) resultByte |= 0x04;
            if (ui->state21_bit3->isChecked()) resultByte |= 0x08;
            if (ui->state21_bit4->isChecked()) resultByte |= 0x10;
            if (ui->state21_bit5->isChecked()) resultByte |= 0x20;
            if (ui->state21_bit6->isChecked()) resultByte |= 0x40;
            if (ui->state21_bit7->isChecked()) resultByte |= 0x80;
            calculatedByte.append(resultByte);
            break;
        case 1:
            if (ui->state23_bit0->isChecked()) resultByte |= 0x01;
            if (ui->state23_bit1->isChecked()) resultByte |= 0x02;
            if (ui->state23_bit2->isChecked()) resultByte |= 0x04;
            if (ui->state23_bit3->isChecked()) resultByte |= 0x08;
            if (ui->state23_bit4->isChecked()) resultByte |= 0x10;
            if (ui->state23_bit5->isChecked()) resultByte |= 0x20;
            if (ui->state23_bit6->isChecked()) resultByte |= 0x40;
            if (ui->state23_bit7->isChecked()) resultByte |= 0x80;
            calculatedByte.append(resultByte);
            resultByte = 0;
            if (ui->state23_bit8->isChecked()) resultByte |= 0x01;
            if (ui->state23_bit9->isChecked()) resultByte |= 0x02;
            if (ui->state23_bit10->isChecked()) resultByte |= 0x04;
            if (ui->state23_bit11->isChecked()) resultByte |= 0x08;
            if (ui->state23_bit12->isChecked()) resultByte |= 0x10;
            if (ui->state23_bit13->isChecked()) resultByte |= 0x20;
            if (ui->state23_bit14->isChecked()) resultByte |= 0x40;
            if (ui->state23_bit15->isChecked()) resultByte |= 0x80;
            calculatedByte.append(resultByte);
    default:
        break;
    }

    return calculatedByte;
}

UCHAR MainWindow::getCurrentTab()
{
    UCHAR result = 0;
    switch (ui->relayStates->currentIndex())
    {
        case 0:
            result = 0x21;
            break;
        case 1:
            result = 0x23;
        default:
            break;
    }
    return result;
}

void MainWindow::editByteForSelected(const UCHAR &stateByte, const QByteArray &byte)
{
    for (const QString& deviceName : selectedDevices)
    {
        qDebug() << deviceName;
        Device* device = iniParser->devices.value(deviceName);

        if (device)
        {
            qDebug () << "In " << device->getPhone() << " changing bytes";
            device->editByte(stateByte, byte);
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
                                                    QApplication::applicationDirPath(),
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
    ipValue->setText(iniParser->gprsSettings["ip"]);
    portValue->setText(iniParser->gprsSettings["port"]);
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
    QList<QCheckBox*> checkBoxes = ui->relayStates->findChildren<QCheckBox*>();
    QList<QPushButton*> pushButtons = ui->relayStates->findChildren<QPushButton*>();
    for (QCheckBox* checkBox : checkBoxes)
    {
        checkBox->setEnabled(checked);
    }
    for (QPushButton* pushButton : pushButtons)
    {
        pushButton->setEnabled(checked);
    }

    // disable auto regen in devices
    for(auto& device : iniParser->devices)
        device->setAutoRegen(!checked);
}

void MainWindow::onSendState23BitsButtonClicked()
{
    editByteForSelected(0x23, calculateByte());
}

void MainWindow::onSendState21BitsButtonClicked()
{
    editByteForSelected(0x21, calculateByte());
}

void MainWindow::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    for (const auto &index : selected.indexes())
    {
        if (index.column() == 1)
        {
            selectedDevices.insert(ui->tableWidget->item(index.row(), 1)->text());
        }
    }

    for (const auto &index : deselected.indexes())
    {
        if (index.column() == 1)
        {
            selectedDevices.remove(ui->tableWidget->item(index.row(), 1)->text());
        }
    }
}


