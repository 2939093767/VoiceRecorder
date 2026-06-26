#include "mainwindow.h"
#include "ui_mainwindow.h"
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_audioRecorder(new AudioRecorder(this))
    , m_speechRecognizer(new SpeechRecognizer(this))
    , m_currentRecordId()
    , m_hypothesisText()
    , m_hasUnsavedChanges(false)
{
    ui->setupUi(this);
    loadStyleSheet();
    setupConnections();
    loadRecords();
    if (!m_records.isEmpty()) {
        loadRecordToEditor(m_records.first());
        ui->historyList->setCurrentRow(0);
    } else {
        addNewRecord();
    }
}

MainWindow::~MainWindow()
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

void MainWindow::setupConnections()
{
    connect(ui->recordBtn, &QPushButton::clicked, this, &MainWindow::onRecordBtnClicked);
    connect(ui->saveBtn, &QPushButton::clicked, this, &MainWindow::onSaveBtnClicked);
    connect(ui->newBtn, &QPushButton::clicked, this, &MainWindow::onNewBtnClicked);
    connect(ui->deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteBtnClicked);
    connect(ui->historyList, &QListWidget::currentRowChanged, this, &MainWindow::onHistoryItemClicked);

    connect(m_audioRecorder, &AudioRecorder::recordingStarted, this, &MainWindow::onRecordingStarted);
    connect(m_audioRecorder, &AudioRecorder::recordingStopped, this, &MainWindow::onRecordingStopped);
    connect(m_audioRecorder, &AudioRecorder::levelChanged, this, &MainWindow::onLevelChanged);
    connect(m_audioRecorder, &AudioRecorder::errorOccurred, this, &MainWindow::onAudioError);

    connect(m_speechRecognizer, &SpeechRecognizer::recognitionResult, this, &MainWindow::onRecognitionResult);
    connect(m_speechRecognizer, &SpeechRecognizer::hypothesisResult, this, &MainWindow::onHypothesisResult);
    connect(m_speechRecognizer, &SpeechRecognizer::listeningStarted, this, &MainWindow::onListeningStarted);
    connect(m_speechRecognizer, &SpeechRecognizer::listeningStopped, this, &MainWindow::onListeningStopped);
    connect(m_speechRecognizer, &SpeechRecognizer::errorOccurred, this, &MainWindow::onSpeechError);

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
    connect(durationTimer, &QTimer::timeout, this, &MainWindow::updateDurationDisplay);
    durationTimer->start();
}

void MainWindow::onRecordBtnClicked()
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

void MainWindow::onSaveBtnClicked()
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

void MainWindow::onNewBtnClicked()
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

void MainWindow::onDeleteBtnClicked()
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

void MainWindow::onRecordingStarted()
{
    ui->recordBtn->setText(QStringLiteral("■ 停止录音"));
    ui->recordBtn->setObjectName("recordBtnRecording");
    ui->recordBtn->style()->unpolish(ui->recordBtn);
    ui->recordBtn->style()->polish(ui->recordBtn);
    setStatusMessage(QStringLiteral("正在录音中..."));
}

void MainWindow::onRecordingStopped()
{
    ui->recordBtn->setText(QStringLiteral("● 开始录音"));
    ui->recordBtn->setObjectName("recordBtn");
    ui->recordBtn->style()->unpolish(ui->recordBtn);
    ui->recordBtn->style()->polish(ui->recordBtn);
    ui->levelBar->setValue(0);
    setStatusMessage(QStringLiteral("录音已停止"));
}

void MainWindow::onLevelChanged(qreal level)
{
    ui->levelBar->setValue(static_cast<int>(level * 100));
}

void MainWindow::onAudioError(const QString &error)
{
    QMessageBox::critical(this, QStringLiteral("录音错误"), error);
    setStatusMessage(QStringLiteral("录音错误"));
}

void MainWindow::onRecognitionResult(const QString &text, bool isFinal)
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

void MainWindow::onHypothesisResult(const QString &text)
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

void MainWindow::onListeningStarted()
{
}

void MainWindow::onListeningStopped()
{
}

void MainWindow::onSpeechError(const QString &error)
{
    setStatusMessage(QStringLiteral("语音识别: %1").arg(error));
}

void MainWindow::onHistoryItemClicked(int row)
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

void MainWindow::restoreCurrentSelection()
{
    for (int i = 0; i < m_records.size(); ++i) {
        if (m_records[i].id == m_currentRecordId) {
            ui->historyList->setCurrentRow(i);
            break;
        }
    }
}

void MainWindow::updateDurationDisplay()
{
    if (m_audioRecorder->isRecording()) {
        qint64 ms = m_audioRecorder->recordDuration();
        ui->durationLabel->setText(formatDuration(ms));
    }
}

void MainWindow::loadRecords()
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

void MainWindow::saveRecords()
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

void MainWindow::addNewRecord()
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

void MainWindow::updateCurrentRecord()
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

void MainWindow::loadRecordToEditor(const RecordItem &record)
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

void MainWindow::clearEditor()
{
    ui->titleEdit->clear();
    ui->contentEdit->clear();
    ui->notesEdit->clear();
    ui->durationLabel->setText(QStringLiteral("00:00:00"));
    ui->levelBar->setValue(0);
    m_hypothesisText.clear();
}

void MainWindow::updateHistoryList()
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

QString MainWindow::generateId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString MainWindow::formatDuration(qint64 ms) const
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

bool MainWindow::confirmDiscard()
{
    auto reply = QMessageBox::question(this, QStringLiteral("未保存的更改"),
        QStringLiteral("当前记录有未保存的更改，是否继续？"),
        QMessageBox::Discard | QMessageBox::Cancel);
    return reply == QMessageBox::Discard;
}

void MainWindow::loadStyleSheet()
{
    QFile file(":/styles/main.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QString::fromUtf8(file.readAll());
        this->setStyleSheet(styleSheet);
        file.close();
    }
}

void MainWindow::setStatusMessage(const QString &message)
{
    ui->statusLabel->setText(message);
    statusBar()->showMessage(message, 3000);
}
