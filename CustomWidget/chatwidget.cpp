#include "chatwidget.h"
#include "qscrollbar.h"
#include <QTimer>
#include <QKeyEvent>
#include <QFontMetrics>

ChatBubbleWidget::ChatBubbleWidget(const QString &text, BubbleRole role, QWidget *parent)
    : QWidget(parent)
    , m_role(role)
    , m_maxWidth(400)
{
    QHBoxLayout *layoutBubble = new QHBoxLayout(this);
    layoutBubble->setContentsMargins(8, 4, 8, 4);
    layoutBubble->setSpacing(0);

    m_labelText = new QLabel(text, this);
    m_labelText->setWordWrap(true);
    m_labelText->setMaximumWidth(m_maxWidth);
    m_labelText->setTextInteractionFlags(Qt::TextSelectableByMouse);

    setupStyle();

    switch (m_role) {
    case User:
        layoutBubble->addStretch(1);
        layoutBubble->addWidget(m_labelText, 0, Qt::AlignRight);
        break;
    case Assistant:
        layoutBubble->addWidget(m_labelText, 0, Qt::AlignLeft);
        layoutBubble->addStretch(1);
        break;
    case System:
        layoutBubble->addStretch(1);
        layoutBubble->addWidget(m_labelText, 0, Qt::AlignCenter);
        layoutBubble->addStretch(1);
        break;
    }
}

ChatBubbleWidget::~ChatBubbleWidget()
{
}

void ChatBubbleWidget::setText(const QString &text)
{
    m_labelText->setText(text);
}

void ChatBubbleWidget::setMaxWidth(int width)
{
    m_maxWidth = width;
    m_labelText->setMaximumWidth(width);
}

QString ChatBubbleWidget::text() const
{
    return m_labelText->text();
}

void ChatBubbleWidget::setupStyle()
{
    QString baseStyle;

    switch (m_role) {
    case User:
        baseStyle = R"(
            QLabel{
                padding: 8px 12px;
                border-radius: 12px;
                font-size: 14px;
                color: #fff;
                background-color: #5ba6f6;
            }
        )";
        break;
    case Assistant:
        baseStyle = R"(
            QLabel{
                padding: 8px 12px;
                border-radius: 12px;
                font-size: 14px;
                color: #333;
                background-color: #e5e5ea;
            }
        )";
        break;
    case System:
        baseStyle = R"(
            QLabel{
                padding: 6px 10px;
                border-radius: 8px;
                font-size: 12px;
                color: #888;
                background-color: transparent;
            }
        )";
        break;
    }

    m_labelText->setStyleSheet(baseStyle);
}

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent)
    , m_bubbleMaxWidth(400)
{
    setupUI();
}

ChatWidget::~ChatWidget()
{
}

void ChatWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(8);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet(R"(
        QScrollArea {
            background-color: transparent;
            border: none;
        }
        QScrollBar:vertical {
            width: 8px;
            background: transparent;
        }
        QScrollBar::handle:vertical {
            background: #ccc;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::handle:hover {
            background: #aaa;
        }
    )");

    m_scrollContent = new QWidget(m_scrollArea);
    m_layoutContent = new QVBoxLayout(m_scrollContent);
    m_layoutContent->setContentsMargins(12, 12, 12, 12);
    m_layoutContent->setSpacing(8);
    m_layoutContent->addStretch(1);

    m_scrollArea->setWidget(m_scrollContent);

    QWidget *inputWidget = new QWidget(this);
    inputWidget->setStyleSheet(R"(
        QWidget {
            background-color: #f0f0f0;
            border-radius: 8px;
        }
    )");
    QHBoxLayout *inputLayout = new QHBoxLayout(inputWidget);
    inputLayout->setContentsMargins(8, 8, 8, 8);
    inputLayout->setSpacing(8);

    m_textEdit = new QTextEdit(inputWidget);
    m_textEdit->setPlaceholderText("输入消息...");
    m_textEdit->setMaximumHeight(100);
    m_textEdit->setStyleSheet(R"(
        QTextEdit {
            background-color: #fff;
            border: 1px solid #ddd;
            border-radius: 6px;
            padding: 8px;
            font-size: 14px;
        }
        QTextEdit:focus {
            border: 1px solid #5ba6f6;
        }
    )");
    m_textEdit->installEventFilter(this);

    m_sendButton = new QPushButton("发送", inputWidget);
    m_sendButton->setFixedSize(70, 36);
    m_sendButton->setCursor(Qt::PointingHandCursor);
    m_sendButton->setStyleSheet(R"(
        QPushButton {
            background-color: #5ba6f6;
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #4a96f5;
        }
        QPushButton:pressed {
            background-color: #3a86e5;
        }
        QPushButton:disabled {
            background-color: #ccc;
        }
    )");

    inputLayout->addWidget(m_textEdit, 1);
    inputLayout->addWidget(m_sendButton, 0, Qt::AlignBottom);

    mainLayout->addWidget(m_scrollArea, 1);
    mainLayout->addWidget(inputWidget, 0, Qt::AlignBottom);

    connect(m_sendButton, &QPushButton::clicked, this, &ChatWidget::onSendClicked);
}

void ChatWidget::addUserMessage(const QString &text)
{
    addMessage(text, ChatBubbleWidget::User);
}

void ChatWidget::addAssistantMessage(const QString &text)
{
    addMessage(text, ChatBubbleWidget::Assistant);
}

void ChatWidget::addSystemMessage(const QString &text)
{
    addMessage(text, ChatBubbleWidget::System);
}

void ChatWidget::addMessage(const QString &text, ChatBubbleWidget::BubbleRole role)
{
    ChatBubbleWidget *bubble = new ChatBubbleWidget(text, role);
    bubble->setMaxWidth(m_bubbleMaxWidth);

    int insertIndex = m_layoutContent->count() - 1;
    m_layoutContent->insertWidget(insertIndex, bubble);

    scrollToBottom();
}

void ChatWidget::clearMessages()
{
    while (m_layoutContent->count() > 1) {
        QLayoutItem *item = m_layoutContent->itemAt(0);
        if (item->widget()) {
            delete item->widget();
        }
        m_layoutContent->removeItem(item);
    }
}

void ChatWidget::setBubbleMaxWidth(int width)
{
    m_bubbleMaxWidth = width;
}

void ChatWidget::setInputPlaceholder(const QString &placeholder)
{
    m_textEdit->setPlaceholderText(placeholder);
}

QString ChatWidget::currentInputText() const
{
    return m_textEdit->toPlainText();
}

void ChatWidget::onSendClicked()
{
    QString text = m_textEdit->toPlainText().trimmed();
    if (!text.isEmpty()) {
        emit sendMessageRequested(text);
        m_textEdit->clear();
    }
}

bool ChatWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_textEdit && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return && !keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {
            onSendClicked();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void ChatWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    scrollToBottom();
}

void ChatWidget::scrollToBottom()
{
    QTimer::singleShot(10, this, [this]() {
        m_scrollArea->verticalScrollBar()->setValue(
            m_scrollArea->verticalScrollBar()->maximum()
        );
    });
}
