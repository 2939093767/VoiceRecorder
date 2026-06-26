#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class AudioRecorder;
class SpeechRecognizer;

struct RecordItem {
    QString id;
    QString title;
    QString content;
    QString notes;
    QDateTime createTime;
    qint64 duration;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRecordBtnClicked();
    void onSaveBtnClicked();
    void onNewBtnClicked();
    void onDeleteBtnClicked();
    void onRecordingStarted();
    void onRecordingStopped();
    void onLevelChanged(qreal level);
    void onAudioError(const QString &error);
    void onRecognitionResult(const QString &text, bool isFinal);
    void onHypothesisResult(const QString &text);
    void onListeningStarted();
    void onListeningStopped();
    void onSpeechError(const QString &error);
    void onHistoryItemClicked(int row);
    void updateDurationDisplay();

private:
    Ui::MainWindow *ui;
    AudioRecorder *m_audioRecorder;
    SpeechRecognizer *m_speechRecognizer;
    QList<RecordItem> m_records;
    QString m_currentRecordId;
    QString m_hypothesisText;
    bool m_hasUnsavedChanges;

    void setupUi();
    void setupConnections();
    void loadStyleSheet();
    void loadRecords();
    void saveRecords();
    void addNewRecord();
    void updateCurrentRecord();
    void loadRecordToEditor(const RecordItem &record);
    void restoreCurrentSelection();
    void clearEditor();
    void updateHistoryList();
    QString generateId() const;
    QString formatDuration(qint64 ms) const;
    bool confirmDiscard();
    void setStatusMessage(const QString &message);
};

#endif
