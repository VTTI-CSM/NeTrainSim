#ifndef ITEMPROPERTYDLG_H
#define ITEMPROPERTYDLG_H

#include <QDialog>
#include "databasetable.h"
#include "columnlist.h"
#include "column.h"
#include "databaserelationship.h"

namespace Ui {
class ItemPropertyDlg;
}

class ItemPropertyDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ItemPropertyDlg(QWidget *parent = 0);
    void showThis( DiagramItem *item);
    ~ItemPropertyDlg();

private:
    Ui::ItemPropertyDlg *ui;
};

#endif // ITEMPROPERTYDLG_H
