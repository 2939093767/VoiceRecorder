#include "chatbubblewidget.h"
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
    QString baseStyle = R"(
        QLabel{
            padding: 8px 12px;
            border-radius: 12px;
            font-size: 14px;
            color: #333;
            background-color: #e5e5ea;
        }
    )";

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
