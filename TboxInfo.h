#ifndef TBOXINFO_H
#define TBOXINFO_H

#include <QDateTime>

typedef struct
{
    qint64  timesTampMs; //QDateTime::currentMSecsSinceEpoch();获取的毫秒级别时间戳
    QString tboxID;
    bool    online;
}tbox_info_t;

#endif // TBOXINFO_H
