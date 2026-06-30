#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include "../models/recorditem.h"

namespace skill_Ability {

/**
 * @brief 会议记录管理器
 * @note 业务能力层，负责记录的增删改查、持久化存储、搜索过滤
 *       纯业务逻辑，无UI依赖，无界面交互
 */
class RecordManager : public QObject
{
    Q_OBJECT

public:
    explicit RecordManager(QObject *parent = nullptr);
    ~RecordManager() override;

    /**
     * @brief 获取所有记录
     * @return 记录列表
     */
    QList<RecordItem> allRecords() const;

    /**
     * @brief 根据ID获取记录
     * @param id 记录ID
     * @return 对应记录，不存在返回空记录
     */
    RecordItem recordById(const QString &id) const;

    /**
     * @brief 添加新记录
     * @param record 记录数据
     * @return 新记录的ID
     */
    QString addRecord(const RecordItem &record);

    /**
     * @brief 更新记录
     * @param id 记录ID
     * @param record 更新后的记录数据
     * @return 是否更新成功
     */
    bool updateRecord(const QString &id, const RecordItem &record);

    /**
     * @brief 删除记录
     * @param id 记录ID
     * @return 是否删除成功
     */
    bool deleteRecord(const QString &id);

    /**
     * @brief 搜索记录（标题、内容、备注）
     * @param keyword 搜索关键词
     * @return 匹配的记录列表
     */
    QList<RecordItem> searchRecords(const QString &keyword) const;

    /**
     * @brief 从磁盘加载记录
     * @return 是否加载成功
     */
    bool loadFromDisk();

    /**
     * @brief 保存记录到磁盘
     * @return 是否保存成功
     */
    bool saveToDisk() const;

    /**
     * @brief 生成唯一ID
     * @return 唯一标识字符串
     */
    QString generateId() const;

signals:
    /**
     * @brief 记录列表变化信号
     * @note 增删改后发出，供UI层刷新
     */
    void recordsChanged();

private:
    QList<RecordItem> m_records;   ///< 记录列表
    QString m_storagePath;          ///< 存储文件路径
};

} // namespace skill_Ability

#endif // RECORDMANAGER_H
