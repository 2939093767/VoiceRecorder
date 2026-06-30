#include "drawerswitchwidget.h"

#include <QHBoxLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QListWidgetItem>
#include <QShowEvent>
#include <QCloseEvent>
#include <QPaintEvent>
#include <QEvent>
#include <QIcon>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QVariantAnimation>
#include <QEasingCurve>
#include <QAbstractAnimation>

namespace {

class DrawerSidebarDelegate : public QStyledItemDelegate
{
public:
    explicit DrawerSidebarDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        painter->save();
        painter->setRenderHint(QPainter::SmoothPixmapTransform);

        // 背景
        if (opt.state & QStyle::State_Selected) {
            painter->fillRect(opt.rect, QColor("#3a3a3a"));
            painter->setPen(QColor("#ffffff"));
        } else if (opt.state & QStyle::State_MouseOver) {
            painter->fillRect(opt.rect, QColor("#2f2f2f"));
            painter->setPen(QColor("#eeeeee"));
        } else {
            painter->setPen(QColor("#eeeeee"));
        }

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QString text = index.data(Qt::DisplayRole).toString();

        const int iconSize = 24;
        const int viewWidth = opt.rect.width();
        const qreal t = qBound(0.0, static_cast<qreal>(viewWidth - 60) / 100.0, 1.0);

        // 图标从居中平滑过渡到左侧
        const int collapsedIconX = opt.rect.left() + (viewWidth - iconSize) / 2;
        const int expandedIconX = opt.rect.left() + 12;
        const int iconX = static_cast<int>(collapsedIconX * (1.0 - t) + expandedIconX * t);
        QRect iconRect(iconX, opt.rect.top() + (opt.rect.height() - iconSize) / 2,
                       iconSize, iconSize);
        QPixmap pixmap = icon.pixmap(iconSize, iconSize,
                                     (opt.state & QStyle::State_Selected) ? QIcon::Selected : QIcon::Normal,
                                     (opt.state & QStyle::State_Enabled) ? QIcon::On : QIcon::Off);
        painter->drawPixmap(iconRect, pixmap);

        // 宽度足够时绘制文字，并随展开渐变透明度
        int textAlpha = qBound(0, (viewWidth - 100) * 255 / 60, 255);
        if (textAlpha > 0 && !text.isEmpty()) {
            QColor textColor = (opt.state & QStyle::State_Selected)
                                   ? QColor("#ffffff")
                                   : QColor("#eeeeee");
            textColor.setAlpha(textAlpha);
            painter->setPen(textColor);

            int textLeft = iconRect.right() + 12;
            QRect textRect(textLeft, opt.rect.top(),
                           opt.rect.right() - textLeft - 12, opt.rect.height());
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
        }

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        Q_UNUSED(option)
        Q_UNUSED(index)
        return QSize(160, 48);
    }
};

} // namespace

DrawerSwitchWidget::DrawerSwitchWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_sidebar = new QListWidget(this);
    m_sidebar->setObjectName("drawerSidebar");
    m_sidebar->setFixedWidth(CollapsedWidth);
    m_sidebar->setFrameShape(QFrame::NoFrame);
    m_sidebar->setIconSize(QSize(24, 24));
    m_sidebar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_sidebar->setItemDelegate(new DrawerSidebarDelegate(m_sidebar));

    m_content = new QStackedWidget(this);
    m_content->setObjectName("drawerContent");

    setStyleSheet(
        "QListWidget#drawerSidebar {"
        "    background-color: #252525;"
        "    border: none;"
        "    outline: 0;"
        "}"
        "QListWidget#drawerSidebar::item {"
        "    border: none;"
        "    padding: 0;"
        "}"
        "QStackedWidget#drawerContent {"
        "    background-color: #1e1e1e;"
        "    border: none;"
        "}"
    );

    mainLayout->addWidget(m_sidebar);
    mainLayout->addWidget(m_content);

    connect(m_sidebar, &QListWidget::currentRowChanged,
            m_content, &QStackedWidget::setCurrentIndex);

    m_expandAnimation = new QVariantAnimation(this);
    m_expandAnimation->setDuration(200);
    m_expandAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_expandAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_sidebar->setFixedWidth(value.toInt());
    });
    m_sidebar->installEventFilter(this);
}

DrawerSwitchWidget::~DrawerSwitchWidget()
{
}

void DrawerSwitchWidget::addOption(const QString &title, QWidget *page)
{
    addOption(QIcon(), title, page);
}

void DrawerSwitchWidget::addOption(const QIcon &icon, const QString &title, QWidget *page)
{
    if (!page)
        return;

    QListWidgetItem *item = new QListWidgetItem(icon, title, m_sidebar);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setSizeHint(QSize(160, 48));
    m_content->addWidget(page);
}

void DrawerSwitchWidget::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_content->count())
        return;
    m_sidebar->setCurrentRow(index);
    m_content->setCurrentIndex(index);
}

int DrawerSwitchWidget::currentIndex() const
{
    return m_content->currentIndex();
}

int DrawerSwitchWidget::count() const
{
    return m_content->count();
}

QWidget *DrawerSwitchWidget::Getwidget(int index) const
{
    if (index < 0 || index >= m_content->count())
        return nullptr;
    return m_content->widget(index);
}

QListWidget *DrawerSwitchWidget::sidebar() const
{
    return m_sidebar;
}

QStackedWidget *DrawerSwitchWidget::content() const
{
    return m_content;
}

void DrawerSwitchWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}

void DrawerSwitchWidget::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
}

void DrawerSwitchWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

bool DrawerSwitchWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_sidebar) {
        if (event->type() == QEvent::Enter) {
            startExpandAnimation(ExpandedWidth);
        } else if (event->type() == QEvent::Leave) {
            startExpandAnimation(CollapsedWidth);
        }
    }
    return QWidget::eventFilter(watched, event);
}

void DrawerSwitchWidget::startExpandAnimation(int endWidth)
{
    if (!m_expandAnimation)
        return;

    if (m_expandAnimation->state() == QAbstractAnimation::Running
        && m_expandAnimation->endValue().toInt() == endWidth) {
        return;
    }

    m_expandAnimation->stop();
    m_expandAnimation->setStartValue(m_sidebar->width());
    m_expandAnimation->setEndValue(endWidth);
    m_expandAnimation->start();
}
