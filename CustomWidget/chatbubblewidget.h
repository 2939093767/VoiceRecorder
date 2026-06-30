#ifndef CHATBUBBLEWIDGET_H
#define CHATBUBBLEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

class ChatBubbleWidget : public QWidget
{
    Q_OBJECT

public:
    enum BubbleRole {
        User,       // 用户消息，右侧蓝色
        Assistant,  // AI/助手消息，左侧灰色
        System      // 系统消息，居中深色
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

#endif // CHATBUBBLEWIDGET_H
