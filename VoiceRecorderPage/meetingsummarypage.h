#ifndef MEETINGSUMMARYPAGE_H
#define MEETINGSUMMARYPAGE_H

#include <QWidget>
#include <QList>
#include "recorditem.h"

class CloudLLM;

namespace Ui {
class MeetingSummaryPage;
}

class MeetingSummaryPage : public QWidget
{
    Q_OBJECT

public:
    explicit MeetingSummaryPage(QWidget *parent = nullptr);
    ~MeetingSummaryPage();

    void loadFromVoiceRecorder(const QString &content, const QString &title);
    void loadFromRecords(const QList<RecordItem> &records);
    bool hasSummaryContent() const;
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
    QString formatRecordsForDisplay(const QList<RecordItem> &records);
    void loadConfig();
    void saveConfig();

private:
    Ui::MeetingSummaryPage *ui;
    CloudLLM *m_cloudLlm = nullptr;
    QString m_originalContent;
    QString m_originalTitle;
    QList<RecordItem> m_records;
};

#endif // MEETINGSUMMARYPAGE_H
