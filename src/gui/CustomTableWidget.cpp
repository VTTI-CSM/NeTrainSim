#include "CustomTableWidget.h"

CustomTableWidget::CustomTableWidget(QWidget *parent)
    : QTableWidget(parent)
{
    // Connect the deleteSelectedRows slot to the custom keyPress signal
    connect(this, &CustomTableWidget::keyPress, this, &CustomTableWidget::deleteSelectedRows);
}

void CustomTableWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        emit keyPress(event);
    }
    else {
        QTableWidget::keyPressEvent(event);
    }
}

void CustomTableWidget::deleteSelectedRows()
{
    QList<QTableWidgetItem*> selectedItems = this->selectedItems();
    if (!selectedItems.isEmpty()) {
        // check if there is only one row
        if (rowCount() <= 1) {
            return;
        }

        // Delete all rows that have a selected item
        for (int i = rowCount() - 1; i >= 0; i--) {
            bool rowSelected = false;
            for (auto item : selectedItems) {
                if (item->row() == i) {
                    if (selectedItems.count() == columnCount()) {
                        rowSelected = true;
                    } else {
                        return;
                    }
                    break;
                }
            }
            if (rowSelected) {
                removeRow(i);
            }
        }
    }
}
