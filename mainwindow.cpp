#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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

        QTableWidgetItem* phoneItem = new QTableWidgetItem(device->getPhone());
        QTableWidgetItem* nameItem = new QTableWidgetItem(device->getName());
        QTableWidgetItem* statusItem = new QTableWidgetItem();
        if (device->isConnected())
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


void MainWindow::on_openIniFileAction_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Выберите ini файл"),
                                                    QDir::homePath(),
                                                    tr("Config files (*.ini)"));
    if (!filePath.isEmpty())
    {
        IniParser iniParser;
        iniParser.parseIniFile(filePath);
        populateDeviceTable(iniParser.devices);
    }
}

