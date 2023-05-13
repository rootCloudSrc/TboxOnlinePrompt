#include "tboxmonitorthread.h"
#include <QEventLoop>
#include <QDateTime>

TboxMonitorThread::TboxMonitorThread(QObject *parent)
{}

void TboxMonitorThread::run()
{
    QNetworkAccessManager manager;
    m_getTokenOk = false;
    QNetworkReply *reply;
    tbox_info_t *tboxInfoTmp = nullptr;

    while(true)
    {
        if(!m_getTokenOk)
        {
             emit tboxGetingStatus(QString("getting token"));
             reply = manager.post(tokenRequest(), tokenReqBody()); //获取token
        }
        else
        {
            tboxInfoTmp = nullptr;
            m_tboxInfoMutex.lock();
            //qDebug() <<"m_tboxInfo size:" <<m_tboxInfo.size();
            if(m_tboxInfo.size())
            {
                tboxInfoTmp = m_tboxInfo[m_tboxInfoVectorIndex++];
                m_tboxInfoVectorIndex %= m_tboxInfo.size();
            }
            m_tboxInfoMutex.unlock();

            if(tboxInfoTmp) //用户在列表输入了tbox id
            {
                qint64 currentTimesTampMs = QDateTime::currentMSecsSinceEpoch();
                //qDebug() <<"currentTimesTampMs:" <<currentTimesTampMs;
                //qDebug() <<"   tboxtimesTampMs:" <<tboxInfoTmp->timesTampMs;
                if((currentTimesTampMs - tboxInfoTmp->timesTampMs) < 60000) //每个ID查询间隔,单位毫秒
                {
                    QThread::sleep(1);
                    continue;
                }
                emit tboxGetingStatus(QString("checking tbox status"));
                emit tboxMonitorThreadLog(QString("checking tbox:").append(tboxInfoTmp->tboxID));
                reply = manager.get(TboxStatusRequest(tboxInfoTmp->tboxID)); //获取盒子状态
                tboxInfoTmp->timesTampMs = currentTimesTampMs;
            }
            else
            {
                QThread::sleep(5);
                continue;
            }
        }

        QEventLoop loop;//循环对象，这个事件循环会在当前线程中运行
        QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();//阻塞，直到事件quit()循环结束或调用了事件循环的 exit() 函数。

        if (reply->error() == QNetworkReply::NoError)
        {
            QByteArray data = reply->readAll();
            // 处理响应内容
            qDebug() <<"get reply data:"<< data << "\r\n";
            parseTokenReplyToAuth(data);
            parseTboxStatus(data);
        }
        else
        {
            // 处理请求错误
            qDebug() <<"get reply error";
            m_getTokenOk = false;
        }
        reply->deleteLater();

        //QThread::sleep(5);
        //QThread::msleep(10);
    }
}

void TboxMonitorThread::tboxInfoClear()
{
    m_tboxInfoMutex.lock();
    for(auto &ptr : m_tboxInfo)
    {
        delete ptr;
        ptr = nullptr;
    }
    m_tboxInfo.clear();// 清空容器
    m_tboxInfo.shrink_to_fit(); // 释放不必要的内存空间 -- 指的是vector对象占用的空间，不是元素内存空间

    m_tboxInfoMutex.unlock();
}

void TboxMonitorThread::tboxInfoAppend(tbox_info_t *info)
{
    if(!info)
        return;

    m_tboxInfoMutex.lock();
    m_tboxInfo.append(info);
    m_tboxInfoMutex.unlock();
}

void TboxMonitorThread::tboxInfoPrint()
{
    m_tboxInfoMutex.lock();
    for(auto &ptr : m_tboxInfo)
    {
        qDebug() << "timesTampMs:" << ptr->timesTampMs
                 << " ,ID:" << ptr->tboxID
                 << " ,online:" << ptr->online;
    }
    m_tboxInfoMutex.unlock();
}

QNetworkRequest TboxMonitorThread::tokenRequest()
{
    QUrl url("https://openapi-oversea.rootcloud.com/account-manage/v1/auth/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return request;
}

QByteArray TboxMonitorThread::tokenReqBody()
{
    QJsonObject json;
    json["grant_type"]    = "password";
    json["username"]      = "18819463378";
    json["password"]      = "1qaz!QAZ";
    json["client_id"]     = "2021112591081ac599593ee6";
    json["client_secret"] = "50bbc13acea1f284ea06187bb4982e79";

    QJsonDocument jsonDoc(json);
    return jsonDoc.toJson();
}

bool TboxMonitorThread::parseTokenReplyToAuth(QByteArray &jsonData)
{
    QString access_token;
    QString token_type;
    QJsonDocument json_doc = QJsonDocument::fromJson(jsonData);
    if (!json_doc.isNull() && json_doc.isObject())
    {
        QJsonObject obj = json_doc.object();
        if (obj.contains("access_token"))
        {
            //QJsonValue access_token_value = obj.value("access_token");
            //if (access_token_value.isString())
            //{
            //    access_token = access_token_value.toString();
            //}
            //QJsonValue token_type = obj.value("token_type");
            //if (token_type.isString())
            //{
            //    token_type = access_token_value.toString();
            //}
            access_token = obj.value("access_token").toString();
            token_type   = obj.value("token_type").toString();
            m_getTboxStatReqHeadAuthorization.clear();
            m_getTboxStatReqHeadAuthorization.append(token_type).append(" ").append(access_token);
            qDebug() << "m_getTboxStatReqHeadAuthorization:" << m_getTboxStatReqHeadAuthorization;
            m_getTokenOk = true;
            return true;
        }
    }

    return false;
}

QNetworkRequest TboxMonitorThread::TboxStatusRequest(QString tboxID)
{
    QString tboxStatURL = QString("https://openapi-oversea.rootcloud.com/thing-instance/v1/device/device-instances/status?_includeMetadata=true&_limit=1000&name=").append(tboxID);
    qDebug() << "TboxStatusRequest:" << tboxStatURL;
    QUrl url(tboxStatURL);
    QNetworkRequest request(url);
//    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", m_getTboxStatReqHeadAuthorization.toUtf8());
    return request;
}

tbox_status_e TboxMonitorThread::parseTboxStatus(QByteArray &jsonData)
{
    //qDebug() << " parseTboxStatus ..........";
    QJsonDocument json_doc = QJsonDocument::fromJson(jsonData);
    if (json_doc.isNull() || !json_doc.isObject()) //非json应答格式
    {
        qDebug() << " parseTboxStatus error data:" << jsonData;
        return TBOX_STATUS_UNKNOW;
    }

    QJsonObject obj = json_doc.object();
    if(obj.contains("message")) //错误原因，有这个字段说明出错了（目前试过的错误原因：1.token超时）
    {
        m_getTokenOk = false;
        qDebug() << " tbox get status faid:" << obj["message"].toString();
        return TBOX_STATUS_UNKNOW;
    }


    if(obj.contains("metadata"))
    {
        QJsonObject metadataObj = obj["metadata"].toObject();
        int totalCount = metadataObj["totalCount"].toInt();
        if(0 == totalCount) //没找着盒子
        {
            qDebug() << " tbox status not found";
            emit tboxNotFound();
            return TBOX_STATUS_NOTFOUND;
        }

        QJsonArray payloadArray = obj["payload"].toArray();
        for (int i = 0; i < payloadArray.size(); ++i)
        {
            QJsonObject payloadObj = payloadArray[i].toObject();

            QString name = payloadObj["name"].toString(); //物实例名称
            bool online = payloadObj["online"].toBool();  //物实例状态
            if(online)
            {
                qDebug() << name << " online";
                emit tboxOnline(name);
            }
            else
            {
                qDebug() << name << " offline";
                emit tboxOffline(name);
            }

        }
    }
    return TBOX_STATUS_UNKNOW;
}


