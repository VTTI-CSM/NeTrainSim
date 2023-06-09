#include "CustomTableWidget.h"
#include "CheckboxDelegate.h"
#include "qstyleditemdelegate.h"

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
    QItemSelectionModel* selectionModel = this->selectionModel();
    if (!selectionModel)
        return;

    QModelIndexList selectedRows = selectionModel->selectedRows();
    if (!selectedRows.isEmpty()) {
        // Check if there is only one row
        if (rowCount() <= 1) {
            emit cannotDeleteRow();
            return;
        }

        // Delete selected rows
        for (const QModelIndex& index : selectedRows) {
            removeRow(index.row());
        }
    }
}

bool CustomTableWidget::hasEmptyCell(const QList<int> &exceptionColumns) {
    for (int row = 0; row < rowCount(); ++row) {
        // if the entire row is empty, it is not counted
        if (isRowEmpty(row, exceptionColumns)) {
            continue;
        }
        for (int column = 0; column < columnCount(); ++column) {
            if (exceptionColumns.contains(column)) {
                continue; // Skip the exception column
            }
            QTableWidgetItem* item = this->item(row, column);
            if (item == nullptr || item->text().isEmpty()) {
                return true; // Found an empty cell
            }
        }
    }
    return false; // No empty cells found
}

bool CustomTableWidget::isRowEmpty(int row, const QList<int>& exceptionColumns) {
    for (int column = 0; column < columnCount(); ++column) {
        if (exceptionColumns.contains(column)) {
            continue;  // Skip the exception columns
        }

        QTableWidgetItem* item = this->item(row, column);
        if (item == nullptr || item->text().isEmpty()) {
            return true;  // Row has a non-empty cell
        }
    }
    return false;  // All cells in the row are empty
}

bool CustomTableWidget::isTableIncomplete(const QList<int>& exceptionColumns) {
    for (int row = 0; row < rowCount(); ++row) {
        bool isRowComplete = true;

        for (int column = 0; column < columnCount(); ++column) {
            if (exceptionColumns.contains(column)) {
                continue;   // Skip the exception columns
            }
            QTableWidgetItem* item = this->item(row, column);

            if (item == nullptr || item->text().isEmpty()) {
                // Found an empty cell or cell with no data in the row
                isRowComplete = false;
                break;
            }
        }

        if (isRowComplete) {
            // At least one complete row is found
            return false;
        }
    }

    // No complete row found
    return true;
}



void CustomTableWidget::setCheckboxDelegateForColumns(int row, const QList<int> &columns) {
    for (auto& column: columns) {
        QCheckBox* checkbox = new QCheckBox(this);

        int cellWidth = this->columnWidth(column);
        int approxCheckboxWidth = 20; // an approximate width of the checkbox, this value might need adjusting
        int margin = (cellWidth - approxCheckboxWidth) / 2;

        checkbox->setStyleSheet(QString("margin-left: %1px; margin-right: %1px;").arg(margin));
        setCellWidget(row, column, checkbox);
    }
}

void CustomTableWidget::setNumericDelegateForColumns(int row, const QList<int> &columns) {

}


void CustomTableWidget::setupTable() {
    for (int column = 0; column < columnCount(); column++) {
        if (itemDelegateForColumn(column) != nullptr) {
            QStyledItemDelegate* delegate = qobject_cast<QStyledItemDelegate*>(itemDelegateForColumn(column));
            if (delegate != nullptr) {
                if (qobject_cast<CheckboxDelegate*>(delegate)) {
                    for (int row = 0; row < rowCount(); row++) {
                        QTableWidgetItem* item = QTableWidget::item(row, column);
                        if (item == nullptr) {
                            setCheckboxDelegateForColumns(row, {column});
                        }
                    }
                }
            }
        }
    }
}

int CustomTableWidget::generateUniqueID() {
    // Find the maximum value in the first column
    int maxID = 0;
    for (int row = 0; row < rowCount(); ++row) {
        QTableWidgetItem* item = this->item(row, 0);
        if (item != nullptr) {
            bool ok;
            int value = item->text().toInt(&ok);
            if (ok && value > maxID) {
                maxID = value;
            }
        }
    }
    return maxID + 1;
}

void CustomTableWidget::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (index.column() == 0) {
        // Retrieve the data from the first column
        QSet<QString> uniqueData;
        for (int row = 0; row < rowCount(); ++row) {
            QTableWidgetItem* item = this->item(row, 0);
            if (item != nullptr) {
                uniqueData.insert(item->text());
            }
        }

        // Check if the new value already exists in the set
        QString newValue = value.toString();
        if (uniqueData.contains(newValue)) {
            return; // Do not set duplicate value
        }
    }

    setData(index, value, role);
}

