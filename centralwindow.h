#ifndef CENTRALWINDOW_H
#define CENTRALWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include "CustomControl/drawerswitchwidget.h"

class VoiceRecorderPage;
class MeetingSummaryPage;

/**
 * @brief 主窗口
 * @note 顶层UI层，负责整体窗口布局、页面切换管理、全局样式加载
 *       仅通过SignalPage页面层间接联动业务，不直接访问Ability底层逻辑
 */
class CentralWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CentralWindow(QWidget *parent = nullptr);
    ~CentralWindow();

private slots:
    void onSwitchToSummary();
    void onPageChanged(int index);
    void onSwitchPage(const QString &pageName);

private:
    void setupUi();
    void createVoiceRecorderPage();
    void createMeetingSummaryPage();
    void loadStyleSheet();

private:
    DrawerSwitchWidget *m_drawerWidget = nullptr;
    VoiceRecorderPage *m_voiceRecorderPage = nullptr;
    MeetingSummaryPage *m_meetingSummaryPage = nullptr;
};

#endif // CENTRALWINDOW_H
