#ifndef LIVE2DACTIONLISTWIDGET_H
#define LIVE2DACTIONLISTWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Live2dActionListWidget; }
QT_END_NAMESPACE

class Live2dActionListItem;

class Live2dActionListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit Live2dActionListWidget(QWidget *parent = nullptr);
    ~Live2dActionListWidget();

    void addItem(const QString &name);
    void removeSelectedItem();
    int currentRow() const;
    int count() const;

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::Live2dActionListWidget *ui;
    QList<Live2dActionListItem*> m_items;
    Live2dActionListItem *m_selectedItem = nullptr;

    void setSelectedItem(Live2dActionListItem *item);
    void relayoutItems();
};

#endif // LIVE2DACTIONLISTWIDGET_H
