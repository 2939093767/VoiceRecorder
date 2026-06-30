#include "voicerecorderpage.h"
#include "ui_voicerecorderpage.h"
#include "../Ability/audiorecorder.h"
#include "../Ability/speechrecognizer.h"
#include "../Ability/recordmanager.h"

#include <QMessageBox>
#include <QDateTime>
#include <QTimer>
#include <QListWidgetItem>
#include <QTextCursor>
#include <QApplication>
#include <QSignalBlocker>
#include <QColor>
#include <QShortcut>
#include <QKeySequence>

VoiceRecorderPage::VoiceRecorderPage(QWidget *parent)
    : PageBase(parent)
    , ui(new Ui::VoiceRecorderPage)
    , m_audioRecorder(nullptr)
    , m_speechRecognizer(nullptr)
    , m_recordManager(nullptr)
    , m_currentRecordId()
    , m_hypothesisText()
    , m_hasUnsavedChanges(false)
    , m_searchKeyword()
{
    ui->setupUi(this);
    initBusinessLayer();
    setupConnections();

    m_recordManager->loadFromDisk();

    QList<RecordItem> records = m_recordManager->allRecords();
    if (!records.isEmpty()) {
        loadRecordToEditor(records.first());
        ui->historyList->setCurrentRow(0);
    } else {
        addNewRecord();
    }
}

VoiceRecorderPage::~VoiceRecorderPage()
{
    if (m_audioRecorder->isRecording()) {
        m_audioRecorder->stopRecording();
    }
    if (m_speechRecognizer->isListening()) {
        m_speechRecognizer->stopListening();
    }
    updateCurrentRecord();
    delete ui;
}

QString VoiceRecorderPage::pageName() const
{
    return QStringLiteral("VoiceRecorderPage");
}

void VoiceRecorderPage::onPageActivated()
{
    updateHistoryList();
}

RecordItem VoiceRecorderPage::currentRecord() const
{
    return m_recordManager->recordById(m_currentRecordId);
}

QList<RecordItem> VoiceRecorderPage::selectedRecords() const
{
    QList<RecordItem> result;
    QList<QListWidgetItem *> items = ui->historyList->selectedItems();
    for (QListWidgetItem *item : items) {
        QString id = item->data(Qt::UserRole).toString();
        RecordItem record = m_recordManager->recordById(id);
        if (!record.id.isEmpty()) {
            result.append(record);
        }
    }
    return result;
}

void VoiceRecorderPage::initBusinessLayer()
{
    m_audioRecorder = new skill_Ability::AudioRecorder(this);
    m_speechRecognizer = new skill_Ability::SpeechRecognizer(this);
    m_recordManager = new skill_Ability::RecordManager(this);
}

void VoiceRecorderPage::setupConnections()
{
    connect(ui->recordBtn, &QPushButton::clicked, this, &VoiceRecorderPage::onRecordBtnClicked);
    connect(ui->saveBtn, &QPushButton::clicked, this, &VoiceRecorderPage::onSaveBtnClicked);
    connect(ui->newBtn, &QPushButton::clicked, this, &VoiceRecorderPage::onNewBtnClicked);
    connect(ui->deleteBtn, &QPushButton::clicked, this, &VoiceRecorderPage::onDeleteBtnClicked);
    connect(ui->summaryBtn, &QPushButton::clicked, this, &VoiceRecorderPage::onSummaryBtnClicked);
    connect(ui->historyList, &QListWidget::currentRowChanged, this, &VoiceRecorderPage::onHistoryItemClicked);
    connect(ui->historyList, &QListWidget::itemSelectionChanged, this, &VoiceRecorderPage::onHistoryItemChanged);
    connect(ui->searchEdit, &QLineEdit::textChanged, this, &VoiceRecorderPage::onSearchTextChanged);

    connect(m_audioRecorder, &skill_Ability::AudioRecorder::recordingStarted,
            this, &VoiceRecorderPage::onRecordingStarted);
    connect(m_audioRecorder, &skill_Ability::AudioRecorder::recordingStopped,
            this, &VoiceRecorderPage::onRecordingStopped);
    connect(m_audioRecorder, &skill_Ability::AudioRecorder::levelChanged,
            this, &VoiceRecorderPage::onLevelChanged);
    connect(m_audioRecorder, &skill_Ability::AudioRecorder::errorOccurred,
            this, &VoiceRecorderPage::onAudioError);

    connect(m_speechRecognizer, &skill_Ability::SpeechRecognizer::recognitionResult,
            this, &VoiceRecorderPage::onRecognitionResult);
    connect(m_speechRecognizer, &skill_Ability::SpeechRecognizer::hypothesisResult,
            this, &VoiceRecorderPage::onHypothesisResult);
    connect(m_speechRecognizer, &skill_Ability::SpeechRecognizer::listeningStarted,
            this, &VoiceRecorderPage::onListeningStarted);
    connect(m_speechRecognizer, &skill_Ability::SpeechRecognizer::listeningStopped,
            this, &VoiceRecorderPage::onListeningStopped);
    connect(m_speechRecognizer, &skill_Ability::SpeechRecognizer::errorOccurred,
            this, &VoiceRecorderPage::onSpeechError);

    connect(m_recordManager, &skill_Ability::RecordManager::recordsChanged,
            this, &VoiceRecorderPage::onRecordsChanged);

    connect(ui->titleEdit, &QLineEdit::textChanged, this, [this]() {
        m_hasUnsavedChanges = true;
    });
    connect(ui->contentEdit, &QTextEdit::textChanged, this, [this]() {
        m_hasUnsavedChanges = true;
    });
    connect(ui->notesEdit, &QTextEdit::textChanged, this, [this]() {
        m_hasUnsavedChanges = true;
    });

    QTimer *durationTimer = new QTimer(this);
    durationTimer->setInterval(100);
    connect(durationTimer, &QTimer::timeout, this, &VoiceRecorderPage::updateDurationDisplay);
    durationTimer->start();

    QShortcut *saveShortcut = new QShortcut(QKeySequence::Save, this);
    saveShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(saveShortcut, &QShortcut::activated, this, &VoiceRecorderPage::onSaveBtnClicked);
}

void VoiceRecorderPage::onRecordBtnClicked()
{
    if (m_audioRecorder->isRecording()) {
        m_audioRecorder->stopRecording();
        m_speechRecognizer->stopListening();
    } else {
        if (!m_speechRecognizer->isAvailable()) {
            QMessageBox::warning(this, QStringLiteral("提示"),
                QStringLiteral("语音识别不可用：%1\n\n将仅进行音频录制，不进行语音识别。")
                    .arg(m_speechRecognizer->lastError()));
        }

        if (m_speechRecognizer->isAvailable()) {
            m_speechRecognizer->startListening();
        }
        m_audioRecorder->startRecording();
    }
}

void VoiceRecorderPage::onSaveBtnClicked()
{
    if (m_currentRecordId.isEmpty()) {
        return;
    }

    QString title = ui->titleEdit->text().trimmed();
    if (title.isEmpty()) {
        RecordItem record = m_recordManager->recordById(m_currentRecordId);
        title = record.createTime.toString(QStringLiteral("yyyy-MM-dd hh:mm"));
        QSignalBlocker blocker(ui->titleEdit);
        ui->titleEdit->setText(title);
    }

    updateCurrentRecord();
    m_hasUnsavedChanges = false;
    setStatusMessage(QStringLiteral("记录已保存：%1").arg(title));
    updateHistoryList();
}

void VoiceRecorderPage::onNewBtnClicked()
{
    if (m_hasUnsavedChanges) {
        if (!confirmDiscard()) {
            return;
        }
    }

    if (m_audioRecorder->isRecording()) {
        m_audioRecorder->stopRecording();
        m_speechRecognizer->stopListening();
    }

    addNewRecord();
}

void VoiceRecorderPage::onDeleteBtnClicked()
{
    if (m_currentRecordId.isEmpty()) {
        return;
    }

    auto reply = QMessageBox::question(this, QStringLiteral("确认删除"),
        QStringLiteral("确定要删除这条记录吗？"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    m_recordManager->deleteRecord(m_currentRecordId);
    clearEditor();

    QList<RecordItem> records = m_recordManager->allRecords();
    if (!records.isEmpty()) {
        loadRecordToEditor(records.first());
        ui->historyList->setCurrentRow(0);
    } else {
        addNewRecord();
    }

    setStatusMessage(QStringLiteral("记录已删除"));
}

void VoiceRecorderPage::onRecordingStarted()
{
    ui->recordBtn->setText(QStringLiteral("■ 停止录音"));
    ui->recordBtn->setObjectName("recordBtnRecording");
    ui->recordBtn->style()->unpolish(ui->recordBtn);
    ui->recordBtn->style()->polish(ui->recordBtn);
    setStatusMessage(QStringLiteral("正在录音中..."));
}

void VoiceRecorderPage::onRecordingStopped()
{
    ui->recordBtn->setText(QStringLiteral("● 开始录音"));
    ui->recordBtn->setObjectName("recordBtn");
    ui->recordBtn->style()->unpolish(ui->recordBtn);
    ui->recordBtn->style()->polish(ui->recordBtn);
    ui->levelBar->setValue(0);
    setStatusMessage(QStringLiteral("录音已停止"));
}

void VoiceRecorderPage::onLevelChanged(qreal level)
{
    ui->levelBar->setValue(static_cast<int>(level * 100));
}

void VoiceRecorderPage::onAudioError(const QString &error)
{
    QMessageBox::critical(this, QStringLiteral("录音错误"), error);
    setStatusMessage(QStringLiteral("录音错误"));
}

void VoiceRecorderPage::onRecognitionResult(const QString &text, bool isFinal)
{
    Q_UNUSED(isFinal);
    if (text.trimmed().isEmpty()) {
        return;
    }

    if (!m_hypothesisText.isEmpty()) {
        QTextCursor cursor = ui->contentEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        int hypoLen = m_hypothesisText.length();
        for (int i = 0; i < hypoLen; ++i) {
            cursor.deletePreviousChar();
        }
        m_hypothesisText.clear();
    }

    ui->contentEdit->insertPlainText(text);
    ui->contentEdit->insertPlainText(QStringLiteral(" "));
    m_hasUnsavedChanges = true;
}

void VoiceRecorderPage::onHypothesisResult(const QString &text)
{
    QTextCursor cursor = ui->contentEdit->textCursor();
    cursor.movePosition(QTextCursor::End);

    if (!m_hypothesisText.isEmpty()) {
        int hypoLen = m_hypothesisText.length();
        for (int i = 0; i < hypoLen; ++i) {
            cursor.deletePreviousChar();
        }
    }

    m_hypothesisText = text;
    ui->contentEdit->insertPlainText(text);
    m_hasUnsavedChanges = true;
}

void VoiceRecorderPage::onListeningStarted()
{
}

void VoiceRecorderPage::onListeningStopped()
{
}

void VoiceRecorderPage::onSpeechError(const QString &error)
{
    setStatusMessage(QStringLiteral("语音识别: %1").arg(error));
}

void VoiceRecorderPage::onSummaryBtnClicked()
{
    QList<RecordItem> records = selectedRecords();
    if (records.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
            QStringLiteral("请先在左侧选择要总结的记录"));
        return;
    }

    emit requestSwitchPage(QStringLiteral("MeetingSummaryPage"));
}

void VoiceRecorderPage::onHistoryItemChanged()
{
    int count = ui->historyList->selectedItems().size();
    if (count > 0) {
        setStatusMessage(QStringLiteral("已选择 %1 条记录").arg(count));
    } else {
        setStatusMessage(QStringLiteral("准备就绪"));
    }
}

void VoiceRecorderPage::onHistoryItemClicked(int row)
{
    QList<RecordItem> records = m_recordManager->allRecords();
    if (row < 0 || row >= records.size()) {
        return;
    }

    const RecordItem &record = records[row];

    if (record.id == m_currentRecordId) {
        return;
    }

    if (m_hasUnsavedChanges) {
        if (!confirmDiscard()) {
            restoreCurrentSelection();
            return;
        }
    }

    if (m_audioRecorder->isRecording()) {
        m_audioRecorder->stopRecording();
        m_speechRecognizer->stopListening();
    }

    loadRecordToEditor(record);
}

void VoiceRecorderPage::restoreCurrentSelection()
{
    QList<RecordItem> records = m_recordManager->allRecords();
    for (int i = 0; i < records.size(); ++i) {
        if (records[i].id == m_currentRecordId) {
            ui->historyList->setCurrentRow(i);
            break;
        }
    }
}

void VoiceRecorderPage::updateDurationDisplay()
{
    if (m_audioRecorder->isRecording()) {
        qint64 ms = m_audioRecorder->recordDuration();
        ui->durationLabel->setText(formatDuration(ms));
    }
}

void VoiceRecorderPage::addNewRecord()
{
    RecordItem item;
    item.title = QString();
    item.content = QString();
    item.notes = QString();
    item.createTime = QDateTime::currentDateTime();
    item.duration = 0;

    QString newId = m_recordManager->addRecord(item);
    m_currentRecordId = newId;
    m_hasUnsavedChanges = false;

    clearEditor();
    updateHistoryList();
    ui->historyList->setCurrentRow(0);
    ui->titleEdit->setFocus();
    setStatusMessage(QStringLiteral("新建记录"));
}

void VoiceRecorderPage::updateCurrentRecord()
{
    RecordItem record = m_recordManager->recordById(m_currentRecordId);
    if (record.id.isEmpty()) {
        return;
    }

    record.title = ui->titleEdit->text().trimmed();
    record.content = ui->contentEdit->toPlainText();
    record.notes = ui->notesEdit->toPlainText();
    if (record.title.isEmpty()) {
        record.title = record.createTime.toString(
            QStringLiteral("yyyy-MM-dd hh:mm"));
    }

    m_recordManager->updateRecord(m_currentRecordId, record);
}

void VoiceRecorderPage::loadRecordToEditor(const RecordItem &record)
{
    m_currentRecordId = record.id;
    m_hypothesisText.clear();

    QSignalBlocker blocker1(ui->titleEdit);
    QSignalBlocker blocker2(ui->contentEdit);
    QSignalBlocker blocker3(ui->notesEdit);

    ui->titleEdit->setText(record.title);
    ui->contentEdit->setPlainText(record.content);
    ui->notesEdit->setPlainText(record.notes);

    m_hasUnsavedChanges = false;

    setStatusMessage(QStringLiteral("已加载记录: %1").arg(record.title));
}

void VoiceRecorderPage::clearEditor()
{
    ui->titleEdit->clear();
    ui->contentEdit->clear();
    ui->notesEdit->clear();
    ui->durationLabel->setText(QStringLiteral("00:00:00"));
    ui->levelBar->setValue(0);
    m_hypothesisText.clear();
}

void VoiceRecorderPage::updateHistoryList()
{
    filterRecords(m_searchKeyword);
}

void VoiceRecorderPage::onRecordsChanged()
{
    updateHistoryList();
}

void VoiceRecorderPage::onSearchTextChanged(const QString &text)
{
    m_searchKeyword = text.trimmed();
    filterRecords(m_searchKeyword);
}

void VoiceRecorderPage::filterRecords(const QString &keyword)
{
    ui->historyList->clear();

    QList<RecordItem> filteredRecords;
    if (keyword.isEmpty()) {
        filteredRecords = m_recordManager->allRecords();
    } else {
        filteredRecords = m_recordManager->searchRecords(keyword);
    }

    for (const RecordItem &item : filteredRecords) {
        QString displayTitle = item.title;
        if (displayTitle.isEmpty()) {
            displayTitle = item.createTime.toString(QStringLiteral("yyyy-MM-dd hh:mm"));
        }

        if (!keyword.isEmpty()) {
            displayTitle = highlightText(displayTitle, keyword);
        }

        QString displayText = QStringLiteral("%1\n%2").arg(
            displayTitle,
            item.createTime.toString(QStringLiteral("yyyy-MM-dd hh:mm")));
        QListWidgetItem *listItem = new QListWidgetItem(displayText);
        listItem->setData(Qt::UserRole, item.id);

        if (!keyword.isEmpty()) {
            listItem->setBackground(QColor(QStringLiteral("#2a2a2a")));
        }

        ui->historyList->addItem(listItem);
    }

    if (!keyword.isEmpty()) {
        setStatusMessage(QStringLiteral("找到 %1 条记录").arg(filteredRecords.size()));
    } else {
        setStatusMessage(QStringLiteral("共 %1 条记录").arg(filteredRecords.size()));
    }
}

QString VoiceRecorderPage::highlightText(const QString &text, const QString &keyword) const
{
    if (keyword.isEmpty()) {
        return text;
    }

    QString result = text;
    int index = result.indexOf(keyword, Qt::CaseInsensitive);
    while (index != -1) {
        result.insert(index + keyword.length(), QStringLiteral("</b>"));
        result.insert(index, QStringLiteral("<b>"));
        index = result.indexOf(keyword, index + keyword.length() + 7, Qt::CaseInsensitive);
    }
    return result;
}

QString VoiceRecorderPage::formatDuration(qint64 ms) const
{
    qint64 totalSeconds = ms / 1000;
    int hours = static_cast<int>(totalSeconds / 3600);
    int minutes = static_cast<int>((totalSeconds % 3600) / 60);
    int seconds = static_cast<int>(totalSeconds % 60);
    return QStringLiteral("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}

bool VoiceRecorderPage::confirmDiscard()
{
    auto reply = QMessageBox::question(this, QStringLiteral("未保存的更改"),
        QStringLiteral("当前记录有未保存的更改，是否继续？"),
        QMessageBox::Discard | QMessageBox::Cancel);
    return reply == QMessageBox::Discard;
}

// void VoiceRecorderPage::setStatusMessage(const QString &message)
// {
//     ui->statusLabel->setText(message);
//     emit statusMessageChanged(message);
// }
