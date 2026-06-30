#ifndef RECORDITEM_H
#define RECORDITEM_H

#include <QString>
#include <QDateTime>

struct RecordItem {
    QString id;
    QString title;
    QString content;
    QString notes;
    QDateTime createTime;
    qint64 duration;
};

#endif // RECORDITEM_H
