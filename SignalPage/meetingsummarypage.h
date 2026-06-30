#ifndef MEETINGSUMMARYPAGE_H
#define MEETINGSUMMARYPAGE_H

#include <QWidget>
#include <QList>
#include "pagebase.h"
#include "../models/recorditem.h"

namespace skill_Ability {
class CloudLLM;
}

namespace Ui {
class MeetingSummaryPage;
}

/**
 * @brief 会议总结页面
 * @note SignalPage页面层，负责会议总结界面展示与交互
 *       承接UI参数、调用Ability层CloudLLM接口、回显执行结果
 *       不编写业务逻辑，仅做数据转发与UI展示
 */
class MeetingSummaryPage : public PageBase
{
    Q_OBJECT

public:
    explicit MeetingSummaryPage(QWidget *parent = nullptr);
    ~MeetingSummaryPage() override;

    QString pageName() const override;
    void onPageActivated() override;

    /**
     * @brief 从语音记录加载内容
     * @param content 录音内容
     * @param title 记录标题
     */
    void loadFromVoiceRecorder(const QString &content, const QString &title);

    /**
     * @brief 从记录列表加载内容
     * @param records 记录列表
     */
    void loadFromRecords(const QList<RecordItem> &records);

    /**
     * @brief 是否有总结内容
     * @return true表示有内容
     */
    bool hasSummaryContent() const;

    /**
     * @brief 设置API密钥
     * @param apiKey API密钥
     */
    void setApiKey(const QString &apiKey);

private slots:
    void onGenerateBtnClicked();
    void onCopyBtnClicked();
    void onClearBtnClicked();
    void onSaveConfigBtnClicked();
    void onStreamReceived(const QString &delta);
    void onLlmFinished(const QString &content);
    void onLlmError(const QString &error);

private:
    QString formatRecordsForDisplay(const QList<RecordItem> &records) const;
    void loadConfig();
    void saveConfig();

private:
    Ui::MeetingSummaryPage *ui;
    skill_Ability::CloudLLM *m_cloudLlm = nullptr;
    QString m_originalContent;
    QString m_originalTitle;
    QList<RecordItem> m_records;
};

#endif // MEETINGSUMMARYPAGE_H
