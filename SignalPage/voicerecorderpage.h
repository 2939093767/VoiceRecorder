#ifndef VOICERECORDERPAGE_H
#define VOICERECORDERPAGE_H

#include <QWidget>
#include <QList>
#include "pagebase.h"
#include "../models/recorditem.h"

namespace skill_Ability {
class AudioRecorder;
class SpeechRecognizer;
class RecordManager;
}

namespace Ui {
class VoiceRecorderPage;
}

/**
 * @brief 语音记录页面
 * @note SignalPage页面层，负责语音记录界面展示与交互
 *       承接UI参数、调用Ability层业务接口、回显执行结果
 *       不编写业务逻辑，仅做数据转发与UI展示
 */
class VoiceRecorderPage : public PageBase
{
    Q_OBJECT

public:
    explicit VoiceRecorderPage(QWidget *parent = nullptr);
    ~VoiceRecorderPage() override;

    QString pageName() const override;
    void onPageActivated() override;

    /**
     * @brief 获取当前编辑的记录
     * @return 当前记录
     */
    RecordItem currentRecord() const;

    /**
     * @brief 获取选中的记录列表
     * @return 选中的记录列表
     */
    QList<RecordItem> selectedRecords() const;


private slots:
    void onRecordBtnClicked();
    void onSaveBtnClicked();
    void onNewBtnClicked();
    void onDeleteBtnClicked();
    void onSummaryBtnClicked();
    void onHistoryItemClicked(int row);
    void onHistoryItemChanged();
    void updateDurationDisplay();
    void onSearchTextChanged(const QString &text);

    void onRecordingStarted();
    void onRecordingStopped();
    void onLevelChanged(qreal level);
    void onAudioError(const QString &error);
    void onRecognitionResult(const QString &text, bool isFinal);
    void onHypothesisResult(const QString &text);
    void onListeningStarted();
    void onListeningStopped();
    void onSpeechError(const QString &error);

    void onRecordsChanged();

    void updateCurrentRecord();
    void loadRecordToEditor(const RecordItem &record);
    void restoreCurrentSelection();
    void clearEditor();
    void updateHistoryList();
    void filterRecords(const QString &keyword);
    QString highlightText(const QString &text, const QString &keyword) const;

private:
    void setupConnections();
    void initBusinessLayer();
    void addNewRecord();
    QString formatDuration(qint64 ms) const;
    bool confirmDiscard();

private:
    Ui::VoiceRecorderPage *ui;

    skill_Ability::AudioRecorder *m_audioRecorder;
    skill_Ability::SpeechRecognizer *m_speechRecognizer;
    skill_Ability::RecordManager *m_recordManager;

    QString m_currentRecordId;
    QString m_hypothesisText;
    bool m_hasUnsavedChanges;
    QString m_searchKeyword;
};

#endif // VOICERECORDERPAGE_H
