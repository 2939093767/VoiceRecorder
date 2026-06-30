#include "live2dactionlistwidget.h"
#include "ui_live2dactionlistwidget.h"

#include <QFrame>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>

class Live2dActionListItem : public QFrame
{
public:
    explicit Live2dActionListItem(const QString &name, QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setFrameShape(QFrame::NoFrame);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        setStyleSheet(QStringLiteral(R"(
            Live2dActionListItem {
                border: 1px solid transparent;
                border-radius: 8px;
                background-color: transparent;
                padding: 4px;
            }
            Live2dActionListItem:hover {
                border: 1px solid palette(highlight);
                background-color: palette(base);
            }
            Live2dActionListItem[selected="true"] {
                border: 2px solid palette(highlight);
                background-color: palette(alternate-base);
            }
        )"));

        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(10, 8, 10, 8);
        layout->setSpacing(10);

        m_label = new QLabel(name, this);
        m_label->setMinimumWidth(60);
        layout->addWidget(m_label);

        m_comboBox = new QComboBox(this);
        m_comboBox->addItem(tr("头部"));
        m_comboBox->addItem(tr("身体"));
        m_comboBox->addItem(tr("全身"));
        m_comboBox->setMinimumWidth(80);
        layout->addWidget(m_comboBox);

        m_lineEdit = new QLineEdit(this);
        m_lineEdit->setPlaceholderText(tr("按键或触控"));
        m_lineEdit->setMinimumWidth(100);
        layout->addWidget(m_lineEdit);

        layout->addStretch();

        m_checkBox = new QCheckBox(tr("禁用"), this);
        layout->addWidget(m_checkBox);
    }

    QString name() const { return m_label->text(); }
    QString triggerArea() const { return m_comboBox->currentText(); }
    QString keyInput() const { return m_lineEdit->text(); }
    bool isDisabled() const { return m_checkBox->isChecked(); }

    void setSelected(bool selected)
    {
        m_selected = selected;
        setProperty("selected", m_selected);
        style()->unpolish(this);
        style()->polish(this);
    }

    bool isSelected() const { return m_selected; }

private:
    QLabel *m_label = nullptr;
    QComboBox *m_comboBox = nullptr;
    QLineEdit *m_lineEdit = nullptr;
    QCheckBox *m_checkBox = nullptr;
    bool m_selected = false;
};

Live2dActionListWidget::Live2dActionListWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Live2dActionListWidget)
{
    ui->setupUi(this);
}

Live2dActionListWidget::~Live2dActionListWidget()
{
    delete ui;
}

void Live2dActionListWidget::addItem(const QString &name)
{
    Live2dActionListItem *item = new Live2dActionListItem(name, this);
    item->installEventFilter(this);

    ui->verticalLayout_items->addWidget(item);
    m_items.append(item);

    if (!m_selectedItem)
        setSelectedItem(item);
}

void Live2dActionListWidget::removeSelectedItem()
{
    if (!m_selectedItem)
        return;

    int index = m_items.indexOf(m_selectedItem);
    if (index < 0)
        return;

    m_items.removeAt(index);

    QLayoutItem *layoutItem = ui->verticalLayout_items->takeAt(index);
    if (layoutItem) {
        QWidget *w = layoutItem->widget();
        delete layoutItem;
        delete w;
    }

    m_selectedItem = nullptr;

    relayoutItems();

    if (!m_items.isEmpty()) {
        int newIndex = qBound(0, index, m_items.count() - 1);
        setSelectedItem(m_items.at(newIndex));
    }
}

int Live2dActionListWidget::currentRow() const
{
    return m_items.indexOf(m_selectedItem);
}

int Live2dActionListWidget::count() const
{
    return m_items.count();
}

void Live2dActionListWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}

void Live2dActionListWidget::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
}

void Live2dActionListWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

bool Live2dActionListWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        Live2dActionListItem *item = static_cast<Live2dActionListItem*>(watched);
        setSelectedItem(item);
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

void Live2dActionListWidget::setSelectedItem(Live2dActionListItem *item)
{
    if (m_selectedItem == item)
        return;

    if (m_selectedItem)
        m_selectedItem->setSelected(false);

    m_selectedItem = item;

    if (m_selectedItem)
        m_selectedItem->setSelected(true);
}

void Live2dActionListWidget::relayoutItems()
{
    // 清空布局（不删除控件）
    QLayoutItem *child;
    while ((child = ui->verticalLayout_items->takeAt(0)) != nullptr) {
        delete child;
    }

    // 重新按单列排列
    for (Live2dActionListItem *item : m_items) {
        ui->verticalLayout_items->addWidget(item);
    }
}
