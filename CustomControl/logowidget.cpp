#include "logowidget.h"
#include "ui_logowidget.h"
#include <QCloseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QShowEvent>

LogoWidget::LogoWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LogoWidget)
{
    ui->setupUi(this);

    setFixedSize(46, 46);

    QPixmap src(":/logo/logo.png");
    if (!src.isNull()) {
        int diameter = qMin(src.width(), src.height());
        QPixmap roundPixmap(diameter, diameter);
        roundPixmap.fill(Qt::transparent);

        QPainter painter(&roundPixmap);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        QPainterPath path;
        path.addEllipse(0, 0, diameter, diameter);
        painter.setClipPath(path);
        painter.drawPixmap((diameter - src.width()) / 2, (diameter - src.height()) / 2, src);
        painter.end();

        ui->label->setPixmap(roundPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

LogoWidget::~LogoWidget()
{
    delete ui;
}

void LogoWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}

void LogoWidget::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
}

void LogoWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}
