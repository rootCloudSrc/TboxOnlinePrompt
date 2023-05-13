#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QThread>
#include <QListWidgetItem>
#include <QListWidget>
#include <QCloseEvent>

#include "tboxmonitorthread.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void tableWidgetTboxIDInit();
    void listWidgetLogInit();
    void startTboxMonitorThread();

private slots:
    void on_pushButton_idRefresh_clicked();

    void on_pushButton_IdClear_clicked();

private:
    Ui::MainWindow *ui;

    int m_tableWidgetTboxIDRows;      //tbox id行数
    QTableWidget *m_tableWidgetTboxID;//tbox id列表
    QListWidget *m_listWidgetLog;     //程序日志输出控件
    QTimer *m_SystemTrayIconTimer;    // 托盘小图标定时器
    QSystemTrayIcon *m_trayIcon;      // 托盘小图标
    bool m_isTrayIconBlinking;
    TboxMonitorThread * m_TboxMonitorThread;

    void listWidgetLogAdditem(QString str);
    void tableWIdgetTboxIdStatisMark(QString tboxID, QString status);
    void trayIconStartBlink();
    void trayIconStopBlink();

protected:
    void closeEvent(QCloseEvent *event); //窗口关闭前执行的时间函数
};
#endif // MAINWINDOW_H
