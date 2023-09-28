#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QInputDialog>
#include <QTableWidget>
#include "iniparser.h"
#include "logger.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static constexpr int DEFAULT_CONNECT_MIN_BOX = 1;
    static constexpr int DEFAULT_CONNECT_SEC_BOX = 0;
    static constexpr int DEFAULT_DISCONNECT_FROM_MIN_BOX = 5;
    static constexpr int DEFAULT_DISCONNECT_FROM_SEC_BOX = 0;
    static constexpr int DEFAULT_DISCONNECT_TO_MIN_BOX = 10;
    static constexpr int DEFAULT_DISCONNECT_TO_SEC_BOX = 0;
    static constexpr int DEFAULT_STATUS_MIN_BOX = 0;
    static constexpr int DEFAULT_STATUS_SEC_BOX = 30;

private:
    Ui::MainWindow *ui;
    std::unique_ptr<Logger> logger;
    std::unique_ptr<IniParser> iniParser;

private:
    void populateDeviceTable(const QMap<QString, Device*> &devices);
    void updateIntervals();

private slots:
    void updateStatus(QTableWidgetItem* item, const QString &status);
    void on_openIniFileAction_triggered();
    void on_connectButton_clicked();
    void on_pushButton_clicked();
    void on_saveValuesCheck_stateChanged(int arg1);
};
#endif // MAINWINDOW_H
