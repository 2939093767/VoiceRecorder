#ifndef SKILLINFOITEM_H
#define SKILLINFOITEM_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class SkillInfoItem; }
QT_END_NAMESPACE

class SkillInfoItem : public QWidget
{
    Q_OBJECT

public:
    explicit SkillInfoItem(QWidget *parent = nullptr);
    ~SkillInfoItem();

    void setLogo(const QIcon &icon);
    void setInfo(const QString &info);
    void setSkillEnabled(bool enabled);
    bool isSkillEnabled() const;

private:
    Ui::SkillInfoItem *ui;
};

#endif // SKILLINFOITEM_H
