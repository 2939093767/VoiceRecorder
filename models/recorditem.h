#ifndef RECORDITEM_H
#define RECORDITEM_H

#include <QString>
#include <QDateTime>

/**
 * @brief 会议记录数据结构
 * @note 纯数据结构体，不包含业务逻辑
 */
struct RecordItem {
    QString id;              ///< 记录唯一标识
    QString title;           ///< 记录标题
    QString content;         ///< 录音转写内容
    QString notes;           ///< 自定义备注文本
    QDateTime createTime;    ///< 创建时间
    qint64 duration;         ///< 录音时长（毫秒）

    RecordItem() : duration(0) {}
};

#endif // RECORDITEM_H
