#ifndef CUSTOMTABLEWIDGET_H
#define CUSTOMTABLEWIDGET_H

#include <QTableWidget>
#include <QKeyEvent>
#include <QSet>

class CustomTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    CustomTableWidget(QWidget *parent = nullptr);
    bool hasEmptyCell(const QList<int>& exceptionColumns = QList<int>());
    bool isRowEmpty(int row, const QList<int>& exceptionColumns = QList<int>());
    void setCheckboxDelegateForColumns(int row, const QList<int>& columns);
    void setNumericDelegateForColumns(int row, const QList<int> &columns);
    void setupTable();
    int generateUniqueID();

signals:
    void keyPress(QKeyEvent *event);
    void cannotDeleteRow();

private slots:
    void deleteSelectedRows() ;


protected:
    void keyPressEvent(QKeyEvent *event) override;
    void setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
};

#endif // CUSTOMTABLEWIDGET_H
