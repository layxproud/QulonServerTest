#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QInputDialog>
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

private:
    Ui::MainWindow *ui;
    IniParser *iniParser;
    Logger *logger;

private:
    void populateDeviceTable(const QMap<QString, Device*> &devices);

private slots:
    void on_openIniFileAction_triggered();
    void on_connectButton_clicked();
    void on_pushButton_clicked();
    void on_connectIntervalBox_valueChanged(int arg1);
    void on_disconnectIntervalBox_valueChanged(int arg1);
};
#endif // MAINWINDOW_H
