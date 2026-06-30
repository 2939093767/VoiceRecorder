#ifndef VOICERECORDERPAGE_H
#define VOICERECORDERPAGE_H

#include <QWidget>
#include <QList>
#include "recorditem.h"

class AudioRecorder;
class SpeechRecognizer;

namespace Ui {
class VoiceRecorderPage;
}

class VoiceRecorderPage : public QWidget
{
    Q_OBJECT

public:
    explicit VoiceRecorderPage(QWidget *parent = nullptr);
    ~VoiceRecorderPage();

    RecordItem currentRecord() const;
    QList<RecordItem> selectedRecords() const;

signals:
    void requestSwitchToSummary();

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

    void updateCurrentRecord();
    void loadRecordToEditor(const RecordItem &record);
    void restoreCurrentSelection();
    void clearEditor();
    void updateHistoryList();
    void filterRecords(const QString &keyword);

private:
    void setupConnections();
    void loadRecords();
    void saveRecords();
    void addNewRecord();
    void setStatusMessage(const QString &message);
    QString generateId() const;
    QString formatDuration(qint64 ms) const;
    bool confirmDiscard();

private:
    Ui::VoiceRecorderPage *ui;

    AudioRecorder *m_audioRecorder;
    SpeechRecognizer *m_speechRecognizer;
    QList<RecordItem> m_records;
    QString m_currentRecordId;
    QString m_hypothesisText;
    bool m_hasUnsavedChanges;
    QString m_searchKeyword;
};

#endif // VOICERECORDERPAGE_H
