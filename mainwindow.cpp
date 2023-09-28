#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , logger(std::make_unique<Logger>(this))
    , iniParser(std::make_unique<IniParser>(logger.get(), this))
{
    ui->setupUi(this);
    logger->setLogWindow(ui->logWindow);

    ui->conIntMinBox->setValue(DEFAULT_CONNECT_MIN_BOX);
    ui->conIntSecBox->setValue(DEFAULT_CONNECT_SEC_BOX);
    ui->discIntFromMinBox->setValue(DEFAULT_DISCONNECT_FROM_MIN_BOX);
    ui->discIntFromSecBox->setValue(DEFAULT_DISCONNECT_FROM_SEC_BOX);
    ui->discIntToMinBox->setValue(DEFAULT_DISCONNECT_TO_MIN_BOX);
    ui->discIntToSecBox->setValue(DEFAULT_DISCONNECT_TO_SEC_BOX);
    ui->statusIntMinBox->setValue(DEFAULT_STATUS_MIN_BOX);
    ui->statusIntSecBox->setValue(DEFAULT_STATUS_SEC_BOX);
}

MainWindow::~MainWindow()
{
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
    for (auto& device : devices)
    {
        QTableWidgetItem* phoneItem = new QTableWidgetItem(device->getPhone());
        QTableWidgetItem* nameItem = new QTableWidgetItem(device->getName());
        QTableWidgetItem* statusItem = new QTableWidgetItem();

        connect(device, &Device::connectionChanged, this, [=](bool status) {
            updateStatus(statusItem, status ? tr("Подключено") : tr("Нет соединения"));
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

void MainWindow::enableSpinBoxes(const bool &arg)
{
    ui->conIntMinBox->setEnabled(arg);
    ui->conIntSecBox->setEnabled(arg);
    ui->discIntFromMinBox->setEnabled(arg);
    ui->discIntFromSecBox->setEnabled(arg);
    ui->discIntToMinBox->setEnabled(arg);
    ui->discIntToSecBox->setEnabled(arg);
    ui->statusIntMinBox->setEnabled(arg);
    ui->statusIntSecBox->setEnabled(arg);
}


void MainWindow::updateStatus(QTableWidgetItem *item, const QString &status)
{
    item->setText(status);
}


void MainWindow::on_openIniFileAction_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Выберите ini файл"),
                                                    QDir::homePath(),
                                                    tr("Config files (*.ini)"));
    if (!filePath.isEmpty())
    {
        if (QFile::exists(filePath))
        {
            iniParser->clearData();
            iniParser->parseIniFile(filePath);
            populateDeviceTable(iniParser->devices);
            ui->ipValue->setText(iniParser->gprsSettings["ip"]);
            ui->portValue->setText(iniParser->gprsSettings["port"]);
        }
        else
        {
            logger->logError(tr("Файл не существует."));
        }
    }
    else
    {
        logger->logError(tr("Ошибка открытия файла."));
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
    for(const auto& device : iniParser->devices)
    {
        device->startConnectionTimer();
    }
}


void MainWindow::on_saveValuesCheck_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        enableSpinBoxes(false);
        updateIntervals();
    }
    else if (arg1 == Qt::Unchecked)
    {
        enableSpinBoxes(true);
    }
}
