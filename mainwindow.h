#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QInputDialog>
#include <QTableWidget>
#include <QSpinBox>
#include "iniparser.h"
#include "logger.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct SpinBoxInfo
{
    QSpinBox* spinBox;
    int defaultValue;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow* ui;
    Logger* logger;
    IniParser* iniParser;

    bool isRunning;
    SpinBoxInfo spinBoxes[8];
    static constexpr int DEFAULT_CONNECT_MIN_BOX = 1;
    static constexpr int DEFAULT_CONNECT_SEC_BOX = 0;
    static constexpr int DEFAULT_DISCONNECT_FROM_MIN_BOX = 5;
    static constexpr int DEFAULT_DISCONNECT_FROM_SEC_BOX = 0;
    static constexpr int DEFAULT_DISCONNECT_TO_MIN_BOX = 10;
    static constexpr int DEFAULT_DISCONNECT_TO_SEC_BOX = 0;
    static constexpr int DEFAULT_STATUS_MIN_BOX = 0;
    static constexpr int DEFAULT_STATUS_SEC_BOX = 30;

private:
    void populateDeviceTable(const QMap<QString, Device*> &devices);
    void updateIntervals();
    void initSpinBoxes();
    void enableSpinBoxes(const bool &arg);

private slots:
    void onConnectButtonClicked();
    void onMultiConnectButtonClicked();
    void onOpenIniFileActionTriggered();
    void onSaveValuesButtonStateChanged(int state);
    void updateDeviceStatus(QTableWidgetItem* item, const QString &status);
};
#endif // MAINWINDOW_H
