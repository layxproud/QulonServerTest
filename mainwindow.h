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

private slots:
    void on_openIniFileAction_triggered();

    void on_connectButton_clicked();

private:
    Ui::MainWindow *ui;
    IniParser *iniParser;
    Logger *logger;

private:
    void populateDeviceTable(const QMap<QString, Device*> &devices);
};
#endif // MAINWINDOW_H
