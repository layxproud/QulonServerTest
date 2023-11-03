#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QInputDialog>
#include <QTableWidget>
#include <QSpinBox>
#include <QLabel>
#include <QButtonGroup>
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

protected:
    void closeEvent(QCloseEvent *event) override; // Declare the override

private:
    Ui::MainWindow* ui;
    Logger* logger;
    IniParser* iniParser;

    bool isRunning;

    // GUI elements
    SpinBoxInfo spinBoxes[10];
    static constexpr int DEFAULT_CONNECT_MIN_BOX = 1;
    static constexpr int DEFAULT_CONNECT_SEC_BOX = 0;
    static constexpr int DEFAULT_DISCONNECT_FROM_MIN_BOX = 5;
    static constexpr int DEFAULT_DISCONNECT_FROM_SEC_BOX = 0;
    static constexpr int DEFAULT_DISCONNECT_TO_MIN_BOX = 10;
    static constexpr int DEFAULT_DISCONNECT_TO_SEC_BOX = 0;
    static constexpr int DEFAULT_SEND_STATUS_MIN_BOX = 0;
    static constexpr int DEFAULT_SEND_STATUS_SEC_BOX = 30;
    static constexpr int DEFAULT_CHANGE_STATUS_MIN_BOX = 0;
    static constexpr int DEFAULT_CHANGE_STATUS_SEC_BOX = 30;

    static constexpr int SEC = 60;
    static constexpr int MILSEC = 1000;

    QLabel* ipLabel;
    QLabel* ipValue;
    QLabel* portLabel;
    QLabel* portValue;

    QButtonGroup relayRadioButtons;
    QButtonGroup logRadioButtons;

    QSet<QString> selectedDevices;

private:
    void initStatusBar();
    void initSpinBoxes();
    void initButtonGroups();
    void populateDeviceTable(const QMap<QString, Device*> &devices);
    void updateDeviceDefaults();
    void enableSpinBoxes(const bool &arg);
    QByteArray calculateByte();
    UCHAR getCurrentTab();
    void editByteForSelected(const UCHAR &stateByte, const QByteArray &byte);
    void selectAllDevices(const int state);

private slots:
    void onConnectButtonClicked();
    void onMultiConnectButtonClicked();
    void onOpenIniFileActionTriggered();
    void onSaveValuesButtonStateChanged(int state);
    void updateDeviceStatus(QTableWidgetItem* item, const QString &status);
    void onRelayManualButtonToggled(bool checked);
    void onSendState21BitsButtonClicked();
    void onSendState23BitsButtonClicked();
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onEnableLogForAllButtonToggled(bool checked);
    void onEnableLogForSelectedButtonToggled(bool checked);
    void onDisableLogButtonToggled(bool checked);
    void onClearLogButtonClicked();
};
#endif // MAINWINDOW_H
