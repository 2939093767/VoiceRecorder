#ifndef HOVERTOOLBAR_H
#define HOVERTOOLBAR_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <QEnterEvent>

class HoverToolBar : public QWidget
{
    Q_OBJECT

public:
    explicit HoverToolBar(QWidget *parent = nullptr);
    ~HoverToolBar();

    void addButton(const QString &text, const QString &iconPath = QString());
    void addButton(QPushButton *button);

    int buttonCount() const;

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    void onAnimTick();

private:
    void initUI();
    void expand();
    void collapse();

    QVBoxLayout *m_mainLayout;
    QWidget *m_triggerWidget;
    QWidget *m_panelWidget;
    QVBoxLayout *m_panelLayout;
    QList<QPushButton*> m_buttons;

    int m_collapsedHeight;
    int m_expandedHeight;
    int m_panelHeight;

    QTimer *m_animTimer;
    int m_animTarget;
    int m_animCurrent;
    int m_animStep;

    bool m_isExpanded;
};

#endif // HOVERTOOLBAR_H
