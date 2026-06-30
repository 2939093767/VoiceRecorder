#include "hovertoolbar.h"

HoverToolBar::HoverToolBar(QWidget *parent)
    : QWidget(parent)
    , m_collapsedHeight(6)
    , m_expandedHeight(0)
    , m_panelHeight(0)
    , m_isExpanded(false)
{
    setAttribute(Qt::WA_Hover);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
    setStyleSheet("QWidget { background-color: transparent; }");
    setFixedWidth(48);
    setFixedHeight(m_collapsedHeight);

    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(16);
    connect(m_animTimer, &QTimer::timeout, this, &HoverToolBar::onAnimTick);

    m_animCurrent = m_collapsedHeight;
    m_animTarget = m_collapsedHeight;
    m_animStep = 0;

    initUI();
}

HoverToolBar::~HoverToolBar()
{
}

void HoverToolBar::initUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // 触发区域
    m_triggerWidget = new QWidget(this);
    m_triggerWidget->setFixedHeight(m_collapsedHeight);
    m_triggerWidget->setStyleSheet(
        "QWidget {"
        "   background-color: rgba(91, 166, 246, 180);"
        "   border-top-left-radius: 4px;"
        "   border-top-right-radius: 4px;"
        "}"
    );

    // 面板区域
    m_panelWidget = new QWidget(this);
    m_panelWidget->setStyleSheet(
        "QWidget {"
        "   background-color: rgba(30, 30, 35, 200);"
        "   border-bottom-left-radius: 8px;"
        "   border-bottom-right-radius: 8px;"
        "}"
    );

    m_panelLayout = new QVBoxLayout(m_panelWidget);
    m_panelLayout->setContentsMargins(4, 8, 4, 8);
    m_panelLayout->setSpacing(6);

    m_mainLayout->addWidget(m_triggerWidget);
    m_mainLayout->addWidget(m_panelWidget);
}

void HoverToolBar::addButton(const QString &text, const QString &iconPath)
{
    QPushButton *btn = new QPushButton(m_panelWidget);
    btn->setFixedSize(40, 40);
    btn->setText(text);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(
        "QPushButton {"
        "   background-color: rgb(70, 70, 75);"
        "   border: none;"
        "   border-radius: 8px;"
        "   color: white;"
        "   font-size: 11px;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgb(91, 166, 246);"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgb(74, 150, 245);"
        "}"
    );

    if (!iconPath.isEmpty()) {
        btn->setIcon(QIcon(iconPath));
        btn->setIconSize(QSize(24, 24));
        btn->setText(QString());
    }

    m_buttons.append(btn);
    m_panelLayout->addWidget(btn);

    m_panelHeight = 16 + m_buttons.count() * 46;
    m_expandedHeight = m_collapsedHeight + m_panelHeight;
}

void HoverToolBar::addButton(QPushButton *button)
{
    button->setParent(m_panelWidget);
    button->setFixedSize(40, 40);
    button->setCursor(Qt::PointingHandCursor);
    button->setStyleSheet(
        "QPushButton {"
        "   background-color: rgb(70, 70, 75);"
        "   border: none;"
        "   border-radius: 8px;"
        "   color: white;"
        "   font-size: 11px;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgb(91, 166, 246);"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgb(74, 150, 245);"
        "}"
    );

    m_buttons.append(button);
    m_panelLayout->addWidget(button);

    m_panelHeight = 16 + m_buttons.count() * 46;
    m_expandedHeight = m_collapsedHeight + m_panelHeight;
}

int HoverToolBar::buttonCount() const
{
    return m_buttons.count();
}

void HoverToolBar::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);
    expand();
}

void HoverToolBar::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    QTimer::singleShot(100, this, &HoverToolBar::collapse);
}

void HoverToolBar::expand()
{
    if (m_isExpanded)
        return;

    m_isExpanded = true;
    m_animTarget = m_expandedHeight;
    m_animStep = 12;
    m_panelWidget->show();
    m_animTimer->start();
}

void HoverToolBar::collapse()
{
    if (!m_isExpanded)
        return;

    m_isExpanded = false;
    m_animTarget = m_collapsedHeight;
    m_animStep = -15;
    m_animTimer->start();
}

void HoverToolBar::onAnimTick()
{
    m_animCurrent += m_animStep;

    if (m_animStep > 0 && m_animCurrent >= m_animTarget) {
        m_animCurrent = m_animTarget;
        m_animTimer->stop();
    } else if (m_animStep < 0 && m_animCurrent <= m_animTarget) {
        m_animCurrent = m_animTarget;
        m_animTimer->stop();
        m_panelWidget->hide();
    }

    setFixedHeight(m_animCurrent);
    m_panelWidget->setFixedHeight(m_animCurrent - m_collapsedHeight);
}
