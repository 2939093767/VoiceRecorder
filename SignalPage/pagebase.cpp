#include "pagebase.h"

PageBase::PageBase(QWidget *parent)
    : QWidget(parent)
{
}

PageBase::~PageBase() = default;

void PageBase::onPageActivated()
{
}

void PageBase::onPageDeactivated()
{
}

void PageBase::setStatusMessage(const QString &message)
{
    Q_UNUSED(message);
}
