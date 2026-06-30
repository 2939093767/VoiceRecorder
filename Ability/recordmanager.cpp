#include "recordmanager.h"

#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUuid>
#include <QStandardPaths>
#include <algorithm>

namespace skill_Ability {

RecordManager::RecordManager(QObject *parent)
    : QObject(parent)
    , m_records()
    , m_storagePath()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!dataDir.isEmpty()) {
        QDir dir(dataDir);
        if (!dir.exists()) {
            dir.mkpath(QStringLiteral("."));
        }
        m_storagePath = dataDir + QStringLiteral("/records.json");
    }
}

RecordManager::~RecordManager()
{
    saveToDisk();
}

QList<RecordItem> RecordManager::allRecords() const
{
    return m_records;
}

RecordItem RecordManager::recordById(const QString &id) const
{
    for (const RecordItem &item : m_records) {
        if (item.id == id) {
            return item;
        }
    }
    return RecordItem();
}

QString RecordManager::addRecord(const RecordItem &record)
{
    RecordItem newRecord = record;
    if (newRecord.id.isEmpty()) {
        newRecord.id = generateId();
    }
    if (newRecord.createTime.isNull()) {
        newRecord.createTime = QDateTime::currentDateTime();
    }
    m_records.prepend(newRecord);
    saveToDisk();
    emit recordsChanged();
    return newRecord.id;
}

bool RecordManager::updateRecord(const QString &id, const RecordItem &record)
{
    for (int i = 0; i < m_records.size(); ++i) {
        if (m_records[i].id == id) {
            m_records[i] = record;
            m_records[i].id = id;
            saveToDisk();
            emit recordsChanged();
            return true;
        }
    }
    return false;
}

bool RecordManager::deleteRecord(const QString &id)
{
    for (int i = 0; i < m_records.size(); ++i) {
        if (m_records[i].id == id) {
            m_records.removeAt(i);
            saveToDisk();
            emit recordsChanged();
            return true;
        }
    }
    return false;
}

QList<RecordItem> RecordManager::searchRecords(const QString &keyword) const
{
    QList<RecordItem> result;
    if (keyword.trimmed().isEmpty()) {
        return m_records;
    }

    for (const RecordItem &item : m_records) {
        bool match = false;
        if (!item.title.isEmpty() && item.title.contains(keyword, Qt::CaseInsensitive)) {
            match = true;
        } else if (!item.content.isEmpty() && item.content.contains(keyword, Qt::CaseInsensitive)) {
            match = true;
        } else if (!item.notes.isEmpty() && item.notes.contains(keyword, Qt::CaseInsensitive)) {
            match = true;
        }
        if (match) {
            result.append(item);
        }
    }
    return result;
}

bool RecordManager::loadFromDisk()
{
    if (m_storagePath.isEmpty()) {
        return false;
    }

    QFile file(m_storagePath);
    if (!file.exists()) {
        return true;
    }

    if (!file.open(QFile::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        return false;
    }

    QJsonArray arr = doc.array();
    m_records.clear();

    for (const QJsonValue &val : arr) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();

        RecordItem item;
        item.id = obj.value(QStringLiteral("id")).toString();
        item.title = obj.value(QStringLiteral("title")).toString();
        item.content = obj.value(QStringLiteral("content")).toString();
        item.notes = obj.value(QStringLiteral("notes")).toString();
        item.createTime = QDateTime::fromString(
            obj.value(QStringLiteral("createTime")).toString(),
            Qt::ISODate);
        item.duration = static_cast<qint64>(obj.value(QStringLiteral("duration")).toDouble());

        if (!item.id.isEmpty()) {
            m_records.append(item);
        }
    }

    std::sort(m_records.begin(), m_records.end(), [](const RecordItem &a, const RecordItem &b) {
        return a.createTime > b.createTime;
    });

    return true;
}

bool RecordManager::saveToDisk() const
{
    if (m_storagePath.isEmpty()) {
        return false;
    }

    QJsonArray arr;
    for (const RecordItem &item : m_records) {
        QJsonObject obj;
        obj[QStringLiteral("id")] = item.id;
        obj[QStringLiteral("title")] = item.title;
        obj[QStringLiteral("content")] = item.content;
        obj[QStringLiteral("notes")] = item.notes;
        obj[QStringLiteral("createTime")] = item.createTime.toString(Qt::ISODate);
        obj[QStringLiteral("duration")] = static_cast<double>(item.duration);
        arr.append(obj);
    }

    QJsonDocument doc(arr);
    QFile file(m_storagePath);
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

QString RecordManager::generateId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

} // namespace skill_Ability
