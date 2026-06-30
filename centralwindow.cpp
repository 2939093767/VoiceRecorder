#include "centralwindow.h"
#include "SignalPage/voicerecorderpage.h"
#include "SignalPage/meetingsummarypage.h"
#include "models/recorditem.h"

#include <QVBoxLayout>
#include <QIcon>
#include <QFile>
#include <QStackedWidget>

CentralWindow::CentralWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
}

CentralWindow::~CentralWindow() = default;

void CentralWindow::setupUi()
{
    setWindowTitle(QStringLiteral("语音记录应用"));
    resize(1200, 800);

    m_drawerWidget = new DrawerSwitchWidget(this);
    setCentralWidget(m_drawerWidget);

    createVoiceRecorderPage();
    createMeetingSummaryPage();

    QStackedWidget *stacked = m_drawerWidget->content();
    if (stacked) {
        connect(stacked, &QStackedWidget::currentChanged, this, &CentralWindow::onPageChanged);
    }

    loadStyleSheet();
}

void CentralWindow::createVoiceRecorderPage()
{
    m_voiceRecorderPage = new VoiceRecorderPage(this);

    QIcon icon(QStringLiteral(":/icons/microphone.png"));
    if (icon.isNull()) {
        icon = QIcon();
    }

    m_drawerWidget->addOption(icon, QStringLiteral("会议语音记录"), m_voiceRecorderPage);
    m_drawerWidget->setCurrentIndex(0);

    connect(m_voiceRecorderPage, &VoiceRecorderPage::requestSwitchPage,
            this, &CentralWindow::onSwitchPage);
}

void CentralWindow::createMeetingSummaryPage()
{
    m_meetingSummaryPage = new MeetingSummaryPage(this);

    QIcon icon(QStringLiteral(":/icons/summary.png"));
    if (icon.isNull()) {
        icon = QIcon();
    }

    m_drawerWidget->addOption(icon, QStringLiteral("会议总结"), m_meetingSummaryPage);
}

void CentralWindow::onPageChanged(int index)
{
    Q_UNUSED(index);
}

void CentralWindow::onSwitchPage(const QString &pageName)
{
    if (pageName == QStringLiteral("MeetingSummaryPage")) {
        onSwitchToSummary();
    }
}

void CentralWindow::onSwitchToSummary()
{
    if (m_voiceRecorderPage && m_meetingSummaryPage) {
        QList<RecordItem> records = m_voiceRecorderPage->selectedRecords();
        if (!records.isEmpty()) {
            m_meetingSummaryPage->loadFromRecords(records);
        } else {
            RecordItem currentRecord = m_voiceRecorderPage->currentRecord();
            m_meetingSummaryPage->loadFromVoiceRecorder(currentRecord.content, currentRecord.title);
        }
        m_drawerWidget->setCurrentIndex(1);
    }
}

void CentralWindow::loadStyleSheet()
{
    QFile file(":/styles/main.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QString::fromUtf8(file.readAll());
        this->setStyleSheet(styleSheet);
        file.close();
    }
}
