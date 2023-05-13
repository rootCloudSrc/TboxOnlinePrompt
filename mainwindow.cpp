#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMetaType>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_isTrayIconBlinking(false)
    , m_SystemTrayIconTimer(nullptr)
{
    ui->setupUi(this);
    this->setWindowIcon(QPixmap(":/Tbox.png"));//左上角图标
    tableWidgetTboxIDInit(); //tbox id表格初始化
    listWidgetLogInit(); //日志输出窗口初始化

    m_trayIcon = new QSystemTrayIcon(QIcon(":/Tbox.png"));// 创建系统托盘对象
    m_trayIcon->setToolTip("Tbox monitor");//鼠标放在图标上面提示的文字
    m_trayIcon->show();//显示图标
#if 0
    m_SystemTrayIconTimer = new QTimer();
    m_SystemTrayIconTimer->setInterval(500);
    m_SystemTrayIconTimer->start();
    // 定时器溢出时，闪烁托盘图标
    QObject::connect(m_SystemTrayIconTimer, &QTimer::timeout, [&]() {
        static bool visible = true;
        if (visible) {
            m_trayIcon->setIcon(QIcon(":/null.png"));
        } else {
            m_trayIcon->setIcon(QIcon(":/Tbox.png"));
        }
        visible = !visible;
    });
#endif
    startTboxMonitorThread();
    qRegisterMetaType<QVector<int>>("QVector<int>");

    qDebug()<<"QSslSocket="<<QSslSocket::sslLibraryBuildVersionString(); //输出当前QT支持的openSSL版本
    qDebug() << "OpenSSL支持情况:" << QSslSocket::supportsSsl();//如果此平台支持SSL，则返回true; 否则，返回false。 如果平台不支持SSL，则套接字将在连接阶段失败:qt.network.ssl: QSslSocket::connectToHostEncrypted: TLS initialization failed
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::tableWidgetTboxIDInit()
{
    QWidget* page = ui->tabWidget->widget(0); //找到第1个页面
    m_tableWidgetTboxID = page->findChild<QTableWidget *>("tableWidget_tboxID");
    QStringList tableList;
    tableList << QString::fromLocal8Bit("实例名称")
              << "";
    m_tableWidgetTboxID->setColumnCount(tableList.length());         //设置列数
    m_tableWidgetTboxID->verticalHeader()->setDefaultSectionSize(25);//设置每列默认高度
    m_tableWidgetTboxID->setHorizontalHeaderLabels(tableList);       //设置表格项

    m_tableWidgetTboxID->setColumnWidth(0, 250);//设置0列宽度
    m_tableWidgetTboxID->setColumnWidth(1, 10);
    m_tableWidgetTboxIDRows = 200;
    for(int i=0; i<m_tableWidgetTboxIDRows; i++)
    {
        m_tableWidgetTboxID->insertRow(i);
    }
    m_tableWidgetTboxID->horizontalHeader()->setStretchLastSection(true);//最后一列始终填充满窗口
    m_tableWidgetTboxID->horizontalHeader()->setStyleSheet("QHeaderView::section{background:white}");//设置表头背景颜色
}

void MainWindow::listWidgetLogInit()
{
    QWidget* page = ui->tabWidget->widget(1); //找到第2个页面
    m_listWidgetLog = page->findChild<QListWidget *>("listWidget_Log");
}

void MainWindow::startTboxMonitorThread()
{
    qDebug() << "startTboxMonitorThread";
    m_TboxMonitorThread = new TboxMonitorThread();
    m_TboxMonitorThread->start();

    connect(m_TboxMonitorThread, &TboxMonitorThread::tboxMonitorThreadLog,[=](QString log){
        listWidgetLogAdditem(log);
    });
    connect(m_TboxMonitorThread, &TboxMonitorThread::tboxGetingStatus,[=](QString status){
        listWidgetLogAdditem(QString("getting tbox status:").append(status));
    });
    connect(m_TboxMonitorThread, &TboxMonitorThread::tboxOnline,[=](QString tboxID){
        listWidgetLogAdditem(QString(" >>> tbox online:").append(tboxID));
        tableWIdgetTboxIdStatisMark(tboxID, QString::fromLocal8Bit("在线"));
    });
    connect(m_TboxMonitorThread, &TboxMonitorThread::tboxOnline, this, &MainWindow::trayIconStartBlink);//右下角托盘图标闪烁--亲测不能用lambda表达式，否则定时器不起作用
    connect(m_TboxMonitorThread, &TboxMonitorThread::tboxOffline,[=](QString tboxID){
        listWidgetLogAdditem(QString(" >>> tbox offline:").append(tboxID));
    });

    connect(m_TboxMonitorThread, &TboxMonitorThread::tboxNotFound,[=](){
        listWidgetLogAdditem(QString(" ########## tbox no found, please check name"));
    });
}


void MainWindow::on_pushButton_idRefresh_clicked()
{
    trayIconStopBlink();
    m_trayIcon->setIcon(QIcon(":/Tbox.png"));//设置托盘图标
    m_tableWidgetTboxID->setStyleSheet("QTableView { background-color: none; }"); //清空所有单元背景颜色

    m_TboxMonitorThread->tboxInfoClear();

    for(int i=0; i<m_tableWidgetTboxIDRows; i++)
    {
        //清空第二列
        if(m_tableWidgetTboxID->item(i,1) !=nullptr &&
           !m_tableWidgetTboxID->item(i,1)->text().isEmpty())
        {
            m_tableWidgetTboxID->item(i,1)->setText("");
        }

        if(m_tableWidgetTboxID->item(i,0)==nullptr ||
           m_tableWidgetTboxID->item(i,0)->text().isEmpty())
        {
            continue;
        }
        m_tableWidgetTboxID->item(i,0)->setBackground(Qt::white); //设置背景颜白色（相当于清空刷新前设置的其他颜色）
        tbox_info_t * tboxInfo = new tbox_info_t();
        tboxInfo->timesTampMs = 0;
        tboxInfo->tboxID = m_tableWidgetTboxID->item(i,0)->text();
        tboxInfo->online = false;
        m_TboxMonitorThread->tboxInfoAppend(tboxInfo);
    }

    m_TboxMonitorThread->tboxInfoPrint();
}

void MainWindow::on_pushButton_IdClear_clicked()
{
    m_tableWidgetTboxID->clear();
    m_TboxMonitorThread->tboxInfoClear();
}

void MainWindow::listWidgetLogAdditem(QString str)
{
    QString logStr(QDateTime::currentDateTime().toString("hh:mm:ss.zzz ").append(str));
    qDebug() << logStr;
    m_listWidgetLog->addItem(logStr);
//    m_listWidgetLog->scrollToBottom();//滚动到最底部--调用这个程序崩溃--未知啥原因
}

void MainWindow::tableWIdgetTboxIdStatisMark(QString tboxID, QString status)
{
    for(int i=0; i<m_tableWidgetTboxIDRows; i++)
    {
        if(m_tableWidgetTboxID->item(i,0)==nullptr ||
           m_tableWidgetTboxID->item(i,0)->text().isEmpty())
        {
            continue;
        }

        if(0 == m_tableWidgetTboxID->item(i,0)->text().compare(tboxID, Qt::CaseInsensitive)) //不区分大小写比较
        {
            m_tableWidgetTboxID->item(i,0)->setBackground(Qt::green);
            m_tableWidgetTboxID->setItem(i, 1, new QTableWidgetItem(status));
            break;
        }
    }
}

void MainWindow::trayIconStartBlink()
{
    if(!m_isTrayIconBlinking)
    {
        qDebug() << "trayIconStartBlink....";
        m_isTrayIconBlinking = true;
        m_SystemTrayIconTimer = new QTimer();
        connect(m_SystemTrayIconTimer, &QTimer::timeout, [=]{
            static bool visible = true;
            if (visible) {
                m_trayIcon->setIcon(QIcon(":/null.png"));
            } else {
                m_trayIcon->setIcon(QIcon(":/Tbox.png"));
            }
            visible = !visible;
        });
        m_SystemTrayIconTimer->start(500); // 每500毫秒切换一次显示状态
    }
}

void MainWindow::trayIconStopBlink()
{
    if(m_isTrayIconBlinking)
    {
        m_isTrayIconBlinking = false;
        m_SystemTrayIconTimer->stop();
        delete m_SystemTrayIconTimer;
        m_SystemTrayIconTimer = nullptr;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "MainWindow closeEvent";
    m_trayIcon->hide();
    delete m_trayIcon;
}
