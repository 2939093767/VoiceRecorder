#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>

class ChatBubbleWidget : public QWidget
{
    Q_OBJECT

public:
    enum BubbleRole {
        User,
        Assistant,
        System
    };
    Q_ENUM(BubbleRole)

    explicit ChatBubbleWidget(const QString &text, BubbleRole role, QWidget *parent = nullptr);
    ~ChatBubbleWidget();

    void setText(const QString &text);
    void setMaxWidth(int width);
    QString text() const;

private:
    void setupStyle();

    QLabel *m_labelText;
    BubbleRole m_role;
    int m_maxWidth;
};

class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(QWidget *parent = nullptr);
    ~ChatWidget();

    void addUserMessage(const QString &text);
    void addAssistantMessage(const QString &text);
    void addSystemMessage(const QString &text);
    void addMessage(const QString &text, ChatBubbleWidget::BubbleRole role);

    void clearMessages();

    void setBubbleMaxWidth(int width);
    void setInputPlaceholder(const QString &placeholder);
    QString currentInputText() const;

signals:
    void sendMessageRequested(const QString &text);

public slots:
    void onSendClicked();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void setupUI();
    void scrollToBottom();

    QVBoxLayout *m_layoutContent;
    QScrollArea *m_scrollArea;
    QWidget *m_scrollContent;
    QTextEdit *m_textEdit;
    QPushButton *m_sendButton;
    int m_bubbleMaxWidth;
};

#endif // CHATWIDGET_H
