#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , logger(new Logger(this))
    , iniParser(new IniParser(logger, this))
    , isRunning(false)
    , selectedDevices{}
    , toggledDevices{}
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
    initButtonGroups();

    // GUI connects
    connect(ui->multiConnectButton, &QPushButton::clicked, this, &MainWindow::onMultiConnectButtonClicked);
    connect(ui->connectButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(ui->saveValuesButton, &QCheckBox::stateChanged, this, &MainWindow::onSaveValuesButtonStateChanged);
    connect(ui->openIniFileAction, &QAction::triggered, this, &MainWindow::onOpenIniFileActionTriggered);
    connect(ui->relayManualButton, &QRadioButton::toggled, this, &MainWindow::onRelayManualButtonToggled);
    connect(ui->sendState21BitsButton, &QPushButton::clicked, this, &MainWindow::onSendStateBitsButtonClicked);
    connect(ui->sendState23BitsButton, &QPushButton::clicked, this, &MainWindow::onSendStateBitsButtonClicked);
    connect(ui->sendState25BitsButton, &QPushButton::clicked, this, &MainWindow::onSendStateBitsButtonClicked);
    connect(ui->deviceTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onSelectionChanged);
    connect(ui->enableLogForAllButton, &QRadioButton::toggled, this, &MainWindow::onEnableLogForAllButtonToggled);
    connect(ui->enableLogForSelectedButton, &QRadioButton::toggled, this, &MainWindow::onEnableLogForSelectedButtonToggled);
    connect(ui->disableLogButton, &QRadioButton::toggled, this, &MainWindow::onDisableLogButtonToggled);
    connect(ui->clearLogButton, &QPushButton::clicked, this, &MainWindow::onClearLogButtonClicked);
    connect(ui->turnOnDevicesButton, &QPushButton::clicked, this, &MainWindow::onTurnOnDevicesButtonClicked);
    connect(ui->turnOffDevicesButton, &QPushButton::clicked, this, &MainWindow::onTurnOffDevicesButtonClicked);
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
    totalDevices = 0;
    numOfConnected = 0;

    ipLabel = new QLabel("IP:", this);
    ipValue = new QLabel("", this);
    portLabel = new QLabel("Port:", this);
    portValue = new QLabel("", this);
    totalDevicesLabel = new QLabel(tr("Всего:"), this);
    totalDevicesValue = new QLabel("0", this);
    numOfConnectedLabel = new QLabel(tr("Подключено:"), this);
    numOfConnectedValue = new QLabel("0", this);

    ui->statusBar->addWidget(ipLabel);
    ui->statusBar->addWidget(ipValue);
    ui->statusBar->addWidget(portLabel);
    ui->statusBar->addWidget(portValue);
    ui->statusBar->addWidget(totalDevicesLabel);
    ui->statusBar->addWidget(totalDevicesValue);
    ui->statusBar->addWidget(numOfConnectedLabel);
    ui->statusBar->addWidget(numOfConnectedValue);
}

void MainWindow::initSpinBoxes()
{
    for (const SpinBoxInfo& info : spinBoxes)
    {
        info.spinBox->setValue(info.defaultValue);
    }
}

void MainWindow::initButtonGroups()
{
    QList<QRadioButton*> relayButtonsList = ui->relayWidget->findChildren<QRadioButton*>();
    QList<QRadioButton*> logButtonsList = ui->logWidget->findChildren<QRadioButton*>();

    for (QRadioButton* button : relayButtonsList)
    {
        relayRadioButtons.addButton(button);
    }
    for (QRadioButton* button : logButtonsList)
    {
        logRadioButtons.addButton(button);
    }
}

void MainWindow::populateDeviceTable(const QMap<QString, Device*> &devices)
{
    ui->deviceTable->clear();
    ui->deviceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->deviceTable->setSelectionMode(QAbstractItemView::ExtendedSelection);

    QStringList headers;
    headers << tr("") << tr("ID") << tr("Имя") << tr("Статус соединения");
    ui->deviceTable->setColumnCount(headers.size());
    ui->deviceTable->setRowCount(devices.size());
    ui->deviceTable->setHorizontalHeaderLabels(headers);
    ui->deviceTable->setAlternatingRowColors(true);

    // Size Policy
    ui->deviceTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->deviceTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->deviceTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->deviceTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    int row = 0;
    for (const auto& device : devices)
    {
        QCheckBox* checkBox = new QCheckBox();
        checkBox->setEnabled(false);
        QTableWidgetItem* phoneItem = new QTableWidgetItem(device->getPhone());
        QTableWidgetItem* nameItem = new QTableWidgetItem(device->getName());
        QTableWidgetItem* statusItem = new QTableWidgetItem(device->isConnected() ? tr("Подключено") : tr("Нет соединения"));

        connect(device, &Device::connectionChanged, this, [=](bool status) {
            status ? (updateDeviceStatus(statusItem, tr("Подключено")), numOfConnected++, numOfConnectedValue->setText(QString::number(numOfConnected))) :
                      (updateDeviceStatus(statusItem, tr("Нет соединения")), numOfConnected--, numOfConnectedValue->setText(QString::number(numOfConnected)));
        });

        ui->deviceTable->setCellWidget(row, 0, checkBox);
        ui->deviceTable->setItem(row, 1, phoneItem);
        ui->deviceTable->setItem(row, 2, nameItem);
        ui->deviceTable->setItem(row, 3, statusItem);

        row++;
        totalDevices++;
    }

    // Header
    QCheckBox *headerCheckBox = new QCheckBox(ui->deviceTable->horizontalHeader());
    int firstColumnWidth = ui->deviceTable->columnWidth(0);
    int headerHeight = ui->deviceTable->horizontalHeader()->height();
    headerCheckBox->setVisible(true);
    headerCheckBox->setGeometry(0, 0, firstColumnWidth, headerHeight);

    connect(ui->deviceTable->horizontalHeader(), &QHeaderView::sectionResized, this, [=](int logicalIndex, int oldSize, int newSize) {
        if (logicalIndex == 0)
        {
            headerCheckBox->setGeometry(0, 0, newSize, headerHeight);
        }
    });

    connect(headerCheckBox, &QCheckBox::stateChanged, this, &MainWindow::selectAllDevices);

    totalDevicesValue->setText(QString::number(totalDevices));
}

void MainWindow::updateDeviceDefaults()
{
    if (toggledDevices.empty())
    {
        logger->logWarning(tr("Ни одно устройство не выделено. Изменения не внесены."));
        return;
    }

    int connectionInterval = ui->conIntMinBox->value() * SEC * MILSEC + ui->conIntSecBox->value() * MILSEC;
    int disconnectionFromInterval = ui->discIntFromMinBox->value() * SEC * MILSEC + ui->discIntFromSecBox->value() * MILSEC;
    int disconnectionToInterval = ui->discIntToMinBox->value() * SEC * MILSEC + ui->discIntToSecBox->value() * MILSEC;
    int sendStatusInterval = ui->sendStatusIntMinBox->value() * SEC * MILSEC + ui->sendStatusIntSecBox->value() * MILSEC;
    int changeStatusInterval = ui->changeStatusIntMinBox->value() * SEC * MILSEC + ui->changeStatusIntSecBox->value() * MILSEC;

    for (const QString& devicePhone : toggledDevices)
    {
        Device* device = iniParser->devices.value(devicePhone);
        device->setConnectionInterval(connectionInterval);
        device->setDisconnectionInterval(disconnectionFromInterval, disconnectionToInterval);
        device->setSendStatusInterval(sendStatusInterval);
        device->setChangeStatusInterval(changeStatusInterval);
        device->setAutoRegen(ui->relayAutoButton->isChecked());
    }
    // editByteForSelected(getCurrentTab(), calculateByte());
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

    // Получаем текущую вкладку
    int tabId = ui->relayStates->currentIndex();
    QWidget* currentTab = ui->relayStates->widget(tabId);

    // Получаем все QCheckBox в текущей вкладке
    QList<QCheckBox*> checkBoxes = currentTab->findChildren<QCheckBox*>();
    std::sort(checkBoxes.begin(), checkBoxes.end(), [](const QCheckBox* a, const QCheckBox* b) {
        return a->objectName() < b->objectName();
    });
    qDebug() << checkBoxes;

    // Обрабатываем каждый QCheckBox
    for (int i = 0; i < checkBoxes.size(); ++i)
    {
        if (checkBoxes[i]->isChecked())
        {
            resultByte |= (1 << (i % 8));
        }

        if ((i + 1) % 8 == 0)
        {
            calculatedByte.append(resultByte);
            resultByte = 0;
        }
    }

    if (checkBoxes.size() % 8 != 0)
    {
        calculatedByte.append(resultByte);
    }

    return calculatedByte;
}

void MainWindow::editByteForSelected(const UCHAR &stateByte, const QByteArray &byte)
{
    if (toggledDevices.isEmpty()) return;

    for (const QString& devicePhone : toggledDevices)
    { 
        Device* device = iniParser->devices.value(devicePhone);
        if (device)
        {
            qDebug () << "In " << device->getPhone() << " changing bytes";
            device->editByte(stateByte, byte);
        }
    }
}

void MainWindow::selectAllDevices(const int state)
{
    if (ui->deviceTable->rowCount() == 0) return;

    if (state == Qt::Checked)
    {
        for (int row = 0; row < ui->deviceTable->rowCount(); ++row)
        {
            toggledDevices.insert(ui->deviceTable->item(row, 1)->text());
        }
    }
    else if (state == Qt::Unchecked)
    {
        toggledDevices.clear();
    }
    updateCheckBoxesFromToggledDevices();
}

void MainWindow::updateCheckBoxesFromToggledDevices()
{
    for (int row = 0; row < ui->deviceTable->rowCount(); ++row)
    {
        QCheckBox* checkBox = qobject_cast<QCheckBox*>(ui->deviceTable->cellWidget(row, 0));

        if (toggledDevices.contains(ui->deviceTable->item(row, 1)->text()))
            checkBox->setChecked(true);
        else
            checkBox->setChecked(false);
    }
}
\
void MainWindow::onConnectButtonClicked()
{
    if (ui->deviceTable->rowCount() <= 0)
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
    if (toggledDevices.empty())
    {
        logger->logWarning(tr("Устройства не найдены!"));
        return;
    }
    if (!isRunning)
    {
        updateDeviceDefaults();
        ui->saveValuesButton->setChecked(true);
        logger->logInfo(tr("Запускаю соединения..."));
        for (const QString& devicePhone : toggledDevices)
        {
            Device* device = iniParser->devices.value(devicePhone);
            if (device)
            {
                qDebug () << "Starting device " << device;
                device->startWork();
            }
        }
        ui->multiConnectButton->setText(tr("СТОП"));
        isRunning = true;
    }
    else
    {
        logger->logInfo(tr("Закрываю соединения..."));
        for (const QString& devicePhone : toggledDevices)
        {
            Device* device = iniParser->devices.value(devicePhone);
            if (device)
            {
                qDebug () << "Stopping device " << device;
                device->stopWork();
            }
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

void MainWindow::onSendStateBitsButtonClicked()
{
    int tabId = ui->relayStates->currentIndex();
    QByteArray byteValue = calculateByte();

    switch (tabId)
    {
        case 0:
        qDebug() << "State 21";
            editByteForSelected(0x21, byteValue);
            break;
        case 1:
            qDebug() << "State 23";
            editByteForSelected(0x23, byteValue);
            break;
        case 2:
            qDebug() << "State 25";
            editByteForSelected(0x25, byteValue);
            break;
        default:
            break;
    }
}

void MainWindow::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    for (const auto &index : selected.indexes())
    {
        if (index.column() == 1)
        {
            selectedDevices.insert(ui->deviceTable->item(index.row(), 1)->text());
            qDebug() << "В список добавлено устройство " << ui->deviceTable->item(index.row(), 1)->text();
            if (ui->enableLogForSelectedButton->isChecked())
            {
                Device* device = iniParser->devices.value(ui->deviceTable->item(index.row(), 1)->text());
                device->editLogStatus(true);
            }
        }
    }

    for (const auto &index : deselected.indexes())
    {
        if (index.column() == 1)
        {
            selectedDevices.remove(ui->deviceTable->item(index.row(), 1)->text());
            qDebug() << "Из списка удалено устройство " << ui->deviceTable->item(index.row(), 1)->text();
            if (ui->enableLogForSelectedButton->isChecked())
            {
                Device* device = iniParser->devices.value(ui->deviceTable->item(index.row(), 1)->text());
                device->editLogStatus(false);
            }
        }
    }
}

void MainWindow::onEnableLogForAllButtonToggled(bool checked)
{
    if (iniParser->devices.isEmpty() || !checked) return;

    for (Device* device : iniParser->devices)
        if (device)
            device->editLogStatus(checked);

    qDebug() << "Включен лог для всех устройств" << checked;
}

void MainWindow::onEnableLogForSelectedButtonToggled(bool checked)
{
    if (selectedDevices.isEmpty() || !checked) return;

    for (Device* device : iniParser->devices)
    {
        if (selectedDevices.contains(device->getPhone()))
            device->editLogStatus(checked);
        else
            device->editLogStatus(!checked);
    }
    qDebug() << "Включен лог для устройств : " << selectedDevices << checked;
}

void MainWindow::onDisableLogButtonToggled(bool checked)
{
    if (iniParser->devices.isEmpty() || !checked) return;

    for (Device* device : iniParser->devices)
        if (device)
            device->editLogStatus(!checked);

    qDebug() << "Выключен лог для всех устройств" << checked;
}

void MainWindow::onClearLogButtonClicked()
{
    ui->logWindow->clear();
}

void MainWindow::onTurnOnDevicesButtonClicked()
{
    if (selectedDevices.empty())
    {
        logger->logWarning(tr("Ни одно устройство не выделено"));
        return;
    }

    toggledDevices.unite(selectedDevices);
    updateCheckBoxesFromToggledDevices();
}

void MainWindow::onTurnOffDevicesButtonClicked()
{
    if (selectedDevices.empty())
    {
        logger->logWarning(tr("Ни одно устройство не выделено"));
        return;
    }

    toggledDevices.subtract(selectedDevices);
    updateCheckBoxesFromToggledDevices();
}


