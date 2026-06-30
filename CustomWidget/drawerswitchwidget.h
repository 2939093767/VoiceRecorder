#ifndef DRAWERSWITCHWIDGET_H
#define DRAWERSWITCHWIDGET_H

#include <QWidget>

class QListWidget;
class QStackedWidget;
class QIcon;
class QVariantAnimation;
class QEvent;

class DrawerSwitchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DrawerSwitchWidget(QWidget *parent = nullptr);
    ~DrawerSwitchWidget();

    void addOption(const QString &title, QWidget *page);
    void addOption(const QIcon &icon, const QString &title, QWidget *page);
    void setCurrentIndex(int index);
    int currentIndex() const;
    int count() const;

    QWidget *Getwidget(int index) const;
    QListWidget *sidebar() const;
    QStackedWidget *content() const;


protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void startExpandAnimation(int endWidth);

    QListWidget *m_sidebar = nullptr;
    QStackedWidget *m_content = nullptr;
    QVariantAnimation *m_expandAnimation = nullptr;

    static constexpr int CollapsedWidth = 60;
    static constexpr int ExpandedWidth = 160;
};

#endif // DRAWERSWITCHWIDGET_H
