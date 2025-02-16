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
#include "calculatebytewidget.h"
#include "lightdeviceswindow.h"
#include "ahpstatewindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Основной GUI класс
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow* ui;
    Logger* logger;
    IniParser* iniParser;
    LightDevicesWindow* lightDevicesWindow;
    AhpStateWindow* ahpStateWindow;

    // Запущен ли сервер
    bool isRunning;

    // Структура для хранения информации о QSpinBox
    struct SpinBoxInfo
    {
        QSpinBox* spinBox;
        int defaultValue;
    };

    SpinBoxInfo spinBoxes[10];

    // Константы QSpinBox
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

    // Константы времени
    static constexpr int SEC = 60;
    static constexpr int MILSEC = 1000;

    // Объекты статус бара
    QLabel* ipLabel;
    QLabel* ipValue;
    QLabel* portLabel;
    QLabel* portValue;
    QLabel* numOfConnectedLabel;
    QLabel* totalDevicesLabel;
    QLabel* numOfConnectedValue;
    QLabel* totalDevicesValue;
    int totalDevices;
    int numOfConnected;

    // Разные группы QRadioButton
    QButtonGroup relayRadioButtons;
    QButtonGroup logRadioButtons;

    // Список выделенных устройств
    QSet<QString> selectedDevices;
    // Список активных устройств
    QSet<QString> toggledDevices;

    QCheckBox *headerCheckBox;

private:
    // Инициализация строки состояния
    void initStatusBar();
    // Инициализация QSpinBox
    void initSpinBoxes();
    // Инициализация групп кнопок
    void initButtonGroups();
    // Инициализация TabWidget
    void initTabWidget();
    // Заполнение таблицы устройств
    void populateDeviceTable(const QMap<QString, Device*> &devices);
    // Обновление значений по умолчанию для устройств
    void updateDeviceDefaults();
    // Включение/отключение QSpinBox
    void enableSpinBoxes(const bool &arg);
    // Вычисление байта на основе QCheckBox
    QByteArray calculateByte();
    // Отредактировать байты для активных устройств
    void editByteForSelected(const UCHAR &stateByte, const QByteArray &byte);
    // Выделить все устройства
    void selectAllDevices(bool state);
    // Обновить чекбоксы состояния у устройств
    void updateCheckBoxesFromToggledDevices();

signals:
    void selectionChanged();

private slots:
    // Обработчики событий и слотов
    void onConnectButtonClicked();
    void onMultiConnectButtonClicked();
    void onSendStateButtonClicked();
    void onOpenIniFileActionTriggered();
    void onSaveValuesButtonStateChanged(int state);
    void updateDeviceStatus(QTableWidgetItem* item, const QString &status);
    void onRelayManualButtonToggled(bool checked);
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onEnableLogForAllButtonToggled(bool checked);
    void onEnableLogForSelectedButtonToggled(bool checked);
    void onDisableLogButtonToggled(bool checked);
    void onClearLogButtonClicked();
    void onTurnOnDevicesButtonClicked();
    void onTurnOffDevicesButtonClicked();
    void onByteCalculated(const QByteArray &byte);
    void onListOfLampsActionTriggered();
    void onAhpStateActionTriggered();
};

#endif // MAINWINDOW_H
