#ifndef CUSTOMTABLEWIDGET_H
#define CUSTOMTABLEWIDGET_H

#include <QTableWidget>
#include <QKeyEvent>

class CustomTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    CustomTableWidget(QWidget *parent = nullptr);

signals:
    void keyPress(QKeyEvent *event);

private slots:
    void deleteSelectedRows() ;

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // CUSTOMTABLEWIDGET_H
