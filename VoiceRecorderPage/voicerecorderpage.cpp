#include "voicerecorderpage.h"
#include "ui_voicerecorderpage.h"
#include "audiorecorder.h"
#include "speechrecognizer.h"

#include <QSettings>
#include <QMessageBox>
#include <QDateTime>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QUuid>
#include <QListWidgetItem>
#include <QTextCursor>
#include <QFile>
#include <QApplication>
#include <QSignalBlocker>
#include <QColor>

VoiceRecorderPage::VoiceRecorderPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VoiceRecorderPage)
    , m_audioRecorder(new AudioRecorder(this))
    , m_speechRecognizer(new SpeechRecognizer(this))
    , m_currentRecordId()
    , m_hypothesisText()
    , m_hasUnsavedChanges(false)
    , m_searchKeyword()
{
    ui->setupUi(this);
    setupConnections();
    loadRecords();
    if (!m_records.isEmpty()) {
        loadRecordToEditor(m_records.first());
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
    saveRecords();
    delete ui;
}

RecordItem VoiceRecorderPage::currentRecord() const
{
    for (const RecordItem &record : m_records) {
        if (record.id == m_currentRecordId) {
            return record;
        }
    }
    return RecordItem();
}

QList<RecordItem> VoiceRecorderPage::selectedRecords() const
{
    QList<RecordItem> result;
    QList<QListWidgetItem *> items = ui->historyList->selectedItems();
    for (QListWidgetItem *item : items) {
        QString id = item->data(Qt::UserRole).toString();
        for (const RecordItem &record : m_records) {
            if (record.id == id) {
                result.append(record);
                break;
            }
        }
    }
    return result;
}

void VoiceRecorderPage::setupConnections()
{
    connect(ui->recordBtn, &QPushButton::clicked, this, &VoiceRecorderPage::onRecordBtnClicked);
    //connect(ui->saveBtn, &QPushButton::clicked, this, &VoiceRecorderPage::onSaveBtnClicked);
    connect(ui->newBtn, &QPushButton::clicked, this, &VoiceRecorderPage::onNewBtnClicked);
    connect(ui->deleteBtn, &QPushButton::clicked, this, &VoiceRecorderPage::onDeleteBtnClicked);
    connect(ui->summaryBtn, &QPushButton::clicked, this, &VoiceRecorderPage::onSummaryBtnClicked);
    connect(ui->historyList, &QListWidget::currentRowChanged, this, &VoiceRecorderPage::onHistoryItemClicked);
    connect(ui->historyList, &QListWidget::itemSelectionChanged, this, &VoiceRecorderPage::onHistoryItemChanged);
    connect(ui->searchEdit, &QLineEdit::textChanged, this, &VoiceRecorderPage::onSearchTextChanged);

    connect(m_audioRecorder, &AudioRecorder::recordingStarted, this, &VoiceRecorderPage::onRecordingStarted);
    connect(m_audioRecorder, &AudioRecorder::recordingStopped, this, &VoiceRecorderPage::onRecordingStopped);
    connect(m_audioRecorder, &AudioRecorder::levelChanged, this, &VoiceRecorderPage::onLevelChanged);
    connect(m_audioRecorder, &AudioRecorder::errorOccurred, this, &VoiceRecorderPage::onAudioError);

    connect(m_speechRecognizer, &SpeechRecognizer::recognitionResult, this, &VoiceRecorderPage::onRecognitionResult);
    connect(m_speechRecognizer, &SpeechRecognizer::hypothesisResult, this, &VoiceRecorderPage::onHypothesisResult);
    connect(m_speechRecognizer, &SpeechRecognizer::listeningStarted, this, &VoiceRecorderPage::onListeningStarted);
    connect(m_speechRecognizer, &SpeechRecognizer::listeningStopped, this, &VoiceRecorderPage::onListeningStopped);
    connect(m_speechRecognizer, &SpeechRecognizer::errorOccurred, this, &VoiceRecorderPage::onSpeechError);

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
}

void VoiceRecorderPage::onRecordBtnClicked()
{
    if (m_audioRecorder->isRecording()) {
        m_audioRecorder->stopRecording();
        m_speechRecognizer->stopListening();
    } else {
        if (!m_speechRecognizer->isAvailable()) {
            QMessageBox::warning(this, QStringLiteral("提示"),
                QStringLiteral("语音识别不可用：%1\n\n将仅进行音频录制，不进行语音识别。").arg(m_speechRecognizer->lastError()));
        }

        if (m_speechRecognizer->isAvailable()) {
            m_speechRecognizer->startListening();
        }
        m_audioRecorder->startRecording();
    }
}

void VoiceRecorderPage::onSaveBtnClicked()
{
    if (ui->titleEdit->text().trimmed().isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请输入记录标题"));
        ui->titleEdit->setFocus();
        return;
    }

    updateCurrentRecord();
    saveRecords();
    m_hasUnsavedChanges = false;
    setStatusMessage(QStringLiteral("记录已保存"));
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

    for (int i = 0; i < m_records.size(); ++i) {
        if (m_records[i].id == m_currentRecordId) {
            m_records.removeAt(i);
            break;
        }
    }

    saveRecords();
    updateHistoryList();
    clearEditor();

    if (!m_records.isEmpty()) {
        loadRecordToEditor(m_records.first());
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
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先在左侧选择要总结的记录"));
        return;
    }

    emit requestSwitchToSummary();
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
    if (row < 0 || row >= m_records.size()) {
        return;
    }

    const RecordItem &record = m_records[row];

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
    for (int i = 0; i < m_records.size(); ++i) {
        if (m_records[i].id == m_currentRecordId) {
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

void VoiceRecorderPage::loadRecords()
{
    QString dataPath = QDir::homePath() + QStringLiteral("/VoiceRecorder/records.json");
    QFile file(dataPath);
    if (!file.exists()) {
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        return;
    }

    QJsonArray arr = doc.array();
    for (const QJsonValue &val : arr) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();
        RecordItem item;
        item.id = obj.value(QStringLiteral("id")).toString();
        item.title = obj.value(QStringLiteral("title")).toString();
        item.content = obj.value(QStringLiteral("content")).toString();
        item.notes = obj.value(QStringLiteral("notes")).toString();
        item.createTime = QDateTime::fromString(
            obj.value(QStringLiteral("createTime")).toString(), Qt::ISODate);
        item.duration = static_cast<qint64>(obj.value(QStringLiteral("duration")).toDouble());
        m_records.append(item);
    }

    updateHistoryList();
}

void VoiceRecorderPage::saveRecords()
{
    QString dirPath = QDir::homePath() + QStringLiteral("/VoiceRecorder");
    QDir dir;
    dir.mkpath(dirPath);

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
    QString dataPath = dirPath + QStringLiteral("/records.json");
    QFile file(dataPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

void VoiceRecorderPage::addNewRecord()
{
    RecordItem item;
    item.id = generateId();
    item.title = QString();
    item.content = QString();
    item.notes = QString();
    item.createTime = QDateTime::currentDateTime();
    item.duration = 0;

    m_records.prepend(item);
    m_currentRecordId = item.id;
    m_hasUnsavedChanges = false;

    clearEditor();
    updateHistoryList();
    ui->historyList->setCurrentRow(0);
    ui->titleEdit->setFocus();
    setStatusMessage(QStringLiteral("新建记录"));
}

void VoiceRecorderPage::updateCurrentRecord()
{
    for (int i = 0; i < m_records.size(); ++i) {
        if (m_records[i].id == m_currentRecordId) {
            m_records[i].title = ui->titleEdit->text().trimmed();
            m_records[i].content = ui->contentEdit->toPlainText();
            m_records[i].notes = ui->notesEdit->toPlainText();
            if (m_records[i].title.isEmpty()) {
                m_records[i].title = m_records[i].createTime.toString(
                    QStringLiteral("yyyy-MM-dd hh:mm"));
            }
            break;
        }
    }
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
    ui->historyList->clear();
    for (const RecordItem &item : m_records) {
        QString displayTitle = item.title;
        if (displayTitle.isEmpty()) {
            displayTitle = item.createTime.toString(QStringLiteral("yyyy-MM-dd hh:mm"));
        }
        QString displayText = QStringLiteral("%1\n%2").arg(
            displayTitle,
            item.createTime.toString(QStringLiteral("yyyy-MM-dd hh:mm")));
        QListWidgetItem *listItem = new QListWidgetItem(displayText);
        listItem->setData(Qt::UserRole, item.id);
        ui->historyList->addItem(listItem);
    }
}

QString VoiceRecorderPage::generateId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
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

void VoiceRecorderPage::onSearchTextChanged(const QString &text)
{
    m_searchKeyword = text.trimmed();
    filterRecords(m_searchKeyword);
}

void VoiceRecorderPage::filterRecords(const QString &keyword)
{
    ui->historyList->clear();

    QList<RecordItem *> filteredRecords;
    for (const RecordItem &item : m_records) {
        if (keyword.isEmpty()) {
            filteredRecords.append(const_cast<RecordItem *>(&item));
        } else {
            bool match = false;
            QString title = item.title;
            QString content = item.content;
            QString notes = item.notes;

            if (!title.isEmpty() && title.contains(keyword, Qt::CaseInsensitive)) {
                match = true;
            } else if (!content.isEmpty() && content.contains(keyword, Qt::CaseInsensitive)) {
                match = true;
            } else if (!notes.isEmpty() && notes.contains(keyword, Qt::CaseInsensitive)) {
                match = true;
            }

            if (match) {
                filteredRecords.append(const_cast<RecordItem *>(&item));
            }
        }
    }

    for (RecordItem *item : filteredRecords) {
        QString displayTitle = item->title;
        if (displayTitle.isEmpty()) {
            displayTitle = item->createTime.toString(QStringLiteral("yyyy-MM-dd hh:mm"));
        }

        if (!keyword.isEmpty()) {
            displayTitle = highlightText(displayTitle, keyword);
        }

        QString displayText = QStringLiteral("%1\n%2").arg(
            displayTitle,
            item->createTime.toString(QStringLiteral("yyyy-MM-dd hh:mm")));
        QListWidgetItem *listItem = new QListWidgetItem(displayText);
        listItem->setData(Qt::UserRole, item->id);

        if (!keyword.isEmpty()) {
            listItem->setBackground(QColor(QStringLiteral("#2a2a2a")));
        }

        ui->historyList->addItem(listItem);
    }

    if (!keyword.isEmpty()) {
        setStatusMessage(QStringLiteral("找到 %1 条记录").arg(filteredRecords.size()));
    } else {
        setStatusMessage(QStringLiteral("共 %1 条记录").arg(m_records.size()));
    }
}

QString VoiceRecorderPage::highlightText(const QString &text, const QString &keyword)
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

void VoiceRecorderPage::setStatusMessage(const QString &message)
{
    ui->statusLabel->setText(message);
}
