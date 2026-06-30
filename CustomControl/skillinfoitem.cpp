#include "skillinfoitem.h"
#include "ui_skillinfoitem.h"

SkillInfoItem::SkillInfoItem(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SkillInfoItem)
{
    ui->setupUi(this);
}

SkillInfoItem::~SkillInfoItem()
{
    delete ui;
}

void SkillInfoItem::setLogo(const QIcon &icon)
{
    if (!icon.isNull()) {
        ui->label_logo->setPixmap(icon.pixmap(ui->label_logo->size()));
    }
}

void SkillInfoItem::setInfo(const QString &info)
{
    ui->label_info->setText(info);
}

void SkillInfoItem::setSkillEnabled(bool enabled)
{
    ui->checkBox_enabled->setChecked(enabled);
}

bool SkillInfoItem::isSkillEnabled() const
{
    return ui->checkBox_enabled->isChecked();
}
