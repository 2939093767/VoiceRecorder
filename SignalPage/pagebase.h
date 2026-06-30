#ifndef PAGEBASE_H
#define PAGEBASE_H

#include <QWidget>
#include <QString>

/**
 * @brief 页面基类
 * @note SignalPage页面层通用基类，所有子页面必须继承此类
 *       定义页面通用接口，统一页面生命周期管理
 */
class PageBase : public QWidget
{
    Q_OBJECT

public:
    explicit PageBase(QWidget *parent = nullptr);
    ~PageBase() override;

    /**
     * @brief 获取页面名称
     * @return 页面名称
     */
    virtual QString pageName() const = 0;

    /**
     * @brief 页面被激活时调用
     * @note 切换到该页面时触发，可用于刷新数据
     */
    virtual void onPageActivated();

    /**
     * @brief 页面被停用时调用
     * @note 离开该页面时触发，可用于保存状态
     */
    virtual void onPageDeactivated();

    /**
     * @brief 设置状态消息
     * @param message 状态消息文本
     */
    virtual void setStatusMessage(const QString &message);

signals:
    /**
     * @brief 请求切换到指定页面
     * @param pageName 目标页面名称
     */
    void requestSwitchPage(const QString &pageName);

    /**
     * @brief 状态消息变化
     * @param message 状态消息
     */
    void statusMessageChanged(const QString &message);
};

#endif // PAGEBASE_H
