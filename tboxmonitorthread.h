#ifndef TBOXMONITORTHREAD_H
#define TBOXMONITORTHREAD_H

#include <QObject>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QByteArray>
#include <QVector>
#include <QMutex>

#include "TboxInfo.h"

typedef enum
{
    TBOX_STATUS_UNKNOW   = 0,
    TBOX_STATUS_NOTFOUND = 1,
    TBOX_STATUS_ONLINE   = 2,
    TBOX_STATUS_OFFLINE  = 3
}tbox_status_e;

class TboxMonitorThread : public QThread
{
    Q_OBJECT

    bool m_getTokenOk; //token获取状态，false：未获取  true：已获取
    QString m_getTboxStatReqHeadAuthorization; //https get获取tbox状态的头部Authorization内容
    int m_tboxInfoVectorIndex = 0;
    QVector<tbox_info_t *> m_tboxInfo;
    QMutex m_tboxInfoMutex;
public:
    explicit TboxMonitorThread(QObject *parent = nullptr);

    void run() override;

    void tboxInfoClear(); //清空m_tboxInfo队列
    void tboxInfoAppend(tbox_info_t *info);//追加m_tboxInfo队列内容
    void tboxInfoPrint();//打印m_tboxInfo队列，调试用

private:
    QNetworkRequest tokenRequest();
    QByteArray tokenReqBody();
    bool parseTokenReplyToAuth(QByteArray &jsonData);

    QNetworkRequest TboxStatusRequest(QString tboxID);
    tbox_status_e parseTboxStatus(QByteArray &jsonData);

signals:
    void tboxMonitorThreadLog(QString log);
    void tboxGetingStatus(QString status);
    void tboxNotFound();
    void tboxOnline(QString tboxID);
    void tboxOffline(QString tboxID);
};

#endif // TBOXMONITORTHREAD_H
