#include "meetingsummarypage.h"
#include "ui_meetingsummarypage.h"
#include "../Ability/cloudllm.h"

#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QSettings>
#include <QComboBox>

MeetingSummaryPage::MeetingSummaryPage(QWidget *parent)
    : PageBase(parent)
    , ui(new Ui::MeetingSummaryPage)
    , m_cloudLlm(new skill_Ability::CloudLLM(this))
    , m_originalContent()
    , m_originalTitle()
    , m_records()
{
    ui->setupUi(this);

    ui->modelCombo->addItem(QStringLiteral("doubao-lite-32k（免费）"), QStringLiteral("doubao-lite-32k"));
    ui->modelCombo->addItem(QStringLiteral("doubao-pro-32k"), QStringLiteral("doubao-pro-32k"));
    ui->modelCombo->addItem(QStringLiteral("doubao-seed-1.6-lite"), QStringLiteral("doubao-seed-1.6-lite"));
    ui->modelCombo->addItem(QStringLiteral("doubao-seed-1.6"), QStringLiteral("doubao-seed-1.6"));

    connect(ui->saveConfigBtn, &QPushButton::clicked, this, &MeetingSummaryPage::onSaveConfigBtnClicked);
    connect(m_cloudLlm, &skill_Ability::CloudLLM::streamReceived,
            this, &MeetingSummaryPage::onStreamReceived);
    connect(m_cloudLlm, &skill_Ability::CloudLLM::finished,
            this, &MeetingSummaryPage::onLlmFinished);
    connect(m_cloudLlm, &skill_Ability::CloudLLM::errorOccurred,
            this, &MeetingSummaryPage::onLlmError);

    loadConfig();
}

MeetingSummaryPage::~MeetingSummaryPage()
{
    delete ui;
}

QString MeetingSummaryPage::pageName() const
{
    return QStringLiteral("MeetingSummaryPage");
}

void MeetingSummaryPage::onPageActivated()
{
}

bool MeetingSummaryPage::hasSummaryContent() const
{
    return !m_originalContent.isEmpty();
}

void MeetingSummaryPage::setApiKey(const QString &apiKey)
{
    if (m_cloudLlm) {
        m_cloudLlm->setApiKey(apiKey);
    }
}

void MeetingSummaryPage::loadFromVoiceRecorder(const QString &content, const QString &title)
{
    m_originalContent = content;
    m_originalTitle = title;
    m_records.clear();

    ui->sourceTitleLabel->setText(title.isEmpty() ? QStringLiteral("无标题") : title);
    ui->sourceContentEdit->setPlainText(content);

    ui->summaryEdit->clear();
    ui->keyPointsEdit->clear();
    ui->actionItemsEdit->clear();

    ui->generateBtn->setEnabled(!content.trimmed().isEmpty());
}

void MeetingSummaryPage::loadFromRecords(const QList<RecordItem> &records)
{
    m_records = records;

    QString displayText = formatRecordsForDisplay(records);
    QString combinedContent;
    for (const RecordItem &record : records) {
        combinedContent += record.content + "\n";
    }
    m_originalContent = combinedContent;

    QString title = records.size() == 1
        ? (records.first().title.isEmpty() ? QStringLiteral("无标题") : records.first().title)
        : QStringLiteral("已选择 %1 条记录").arg(records.size());
    m_originalTitle = title;

    ui->sourceTitleLabel->setText(title);
    ui->sourceContentEdit->setPlainText(displayText);

    ui->summaryEdit->clear();
    ui->keyPointsEdit->clear();
    ui->actionItemsEdit->clear();

    ui->generateBtn->setEnabled(!combinedContent.trimmed().isEmpty());
}

QString MeetingSummaryPage::formatRecordsForDisplay(const QList<RecordItem> &records) const
{
    QString result;
    for (int i = 0; i < records.size(); ++i) {
        const RecordItem &record = records[i];
        QString title = record.title.isEmpty()
            ? record.createTime.toString(QStringLiteral("yyyy-MM-dd hh:mm"))
            : record.title;
        result += QStringLiteral("【%1】%2\n").arg(i + 1).arg(title);
        result += QStringLiteral("—— 录音 ——\n");
        result += record.content.isEmpty() ? QStringLiteral("(无录音内容)") : record.content;
        result += QStringLiteral("\n—— 自定义文本 ——\n");
        result += record.notes.isEmpty() ? QStringLiteral("(无备注)") : record.notes;
        result += QStringLiteral("\n\n");
    }
    return result.trimmed();
}

void MeetingSummaryPage::onGenerateBtnClicked()
{
    if (m_originalContent.trimmed().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("没有可总结的内容"));
        return;
    }

    ui->generateBtn->setEnabled(false);
    ui->generateBtn->setText(QStringLiteral("生成中..."));
    ui->summaryEdit->clear();

    QString prompt = QStringLiteral("帮我生成一篇会议总结，以下是会议记录内容：\n\n%1").arg(m_originalContent);
    m_cloudLlm->chat(prompt);
}

void MeetingSummaryPage::onStreamReceived(const QString &delta)
{
    ui->summaryEdit->insertPlainText(delta);
}

void MeetingSummaryPage::onLlmFinished(const QString &content)
{
    Q_UNUSED(content);
    ui->generateBtn->setEnabled(true);
    ui->generateBtn->setText(QStringLiteral("生成总结"));
}

void MeetingSummaryPage::onLlmError(const QString &error)
{
    ui->generateBtn->setEnabled(true);
    ui->generateBtn->setText(QStringLiteral("生成总结"));
    ui->summaryEdit->setPlainText(QStringLiteral("生成失败：%1").arg(error));
}

void MeetingSummaryPage::onCopyBtnClicked()
{
    QString summary = ui->summaryEdit->toPlainText();
    QString keyPoints = ui->keyPointsEdit->toPlainText();
    QString actionItems = ui->actionItemsEdit->toPlainText();

    if (summary.isEmpty() && keyPoints.isEmpty() && actionItems.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("没有内容可复制"));
        return;
    }

    QString fullSummary;
    fullSummary += QStringLiteral("【会议总结】\n");
    fullSummary += summary;
    fullSummary += QStringLiteral("\n\n【关键要点】\n");
    fullSummary += keyPoints;
    fullSummary += QStringLiteral("\n\n【行动项】\n");
    fullSummary += actionItems;

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(fullSummary);

    QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("已复制到剪贴板"));
}

void MeetingSummaryPage::onClearBtnClicked()
{
    ui->summaryEdit->clear();
    ui->keyPointsEdit->clear();
    ui->actionItemsEdit->clear();
    ui->generateBtn->setEnabled(!m_originalContent.trimmed().isEmpty());
}

void MeetingSummaryPage::onSaveConfigBtnClicked()
{
    QString apiKey = ui->apiKeyEdit->text().trimmed();
    if (apiKey.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请输入 API Key"));
        ui->apiKeyEdit->setFocus();
        return;
    }

    QString model = ui->modelCombo->currentData().toString();
    m_cloudLlm->setApiKey(apiKey);
    m_cloudLlm->setModel(model);

    saveConfig();
    QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("配置已保存"));
}

void MeetingSummaryPage::loadConfig()
{
    QSettings settings(QStringLiteral("VoiceRecorder"), QStringLiteral("CloudLLM"));
    QString apiKey = settings.value(QStringLiteral("apiKey"), QString()).toString();
    QString model = settings.value(QStringLiteral("model"), QStringLiteral("doubao-lite-32k")).toString();

    ui->apiKeyEdit->setText(apiKey);

    int idx = ui->modelCombo->findData(model);
    if (idx >= 0) {
        ui->modelCombo->setCurrentIndex(idx);
    }

    if (!apiKey.isEmpty()) {
        m_cloudLlm->setApiKey(apiKey);
        m_cloudLlm->setModel(model);
    }
}

void MeetingSummaryPage::saveConfig()
{
    QSettings settings(QStringLiteral("VoiceRecorder"), QStringLiteral("CloudLLM"));
    settings.setValue(QStringLiteral("apiKey"), ui->apiKeyEdit->text().trimmed());
    settings.setValue(QStringLiteral("model"), ui->modelCombo->currentData().toString());
}
