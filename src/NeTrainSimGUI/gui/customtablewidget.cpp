#include "customtablewidget.h"
#include "gui/checkboxdelegate.h"
#include "gui/comboboxdelegate.h"
#include "gui/intnumericdelegate.h"
#include "gui/nonemptydelegate.h"
#include "gui/numericdelegate.h"
#include "qapplication.h"
//#include "checkboxdelegate.h"
//#include "qstyleditemdelegate.h"
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QMessageBox>
#include "../NeTrainSim/util/utils.h"

CustomTableWidget::CustomTableWidget(QWidget *parent)
    : QTableWidget(parent)
{
    // Connect the deleteSelectedRows slot to the custom keyPress signal
    connect(this, &CustomTableWidget::keyPress, this,
            &CustomTableWidget::deleteSelectedRows);
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

void CustomTableWidget::contextMenuEvent(QContextMenuEvent *event) {
    QMenu contextMenu(this);

    // Add other actions here...

    QAction importAction("Clear and Import from Clipboard", this);
    QAction clearAction("Clear Table", this);

    connect(&importAction, &QAction::triggered, this,
            &CustomTableWidget::importFromClipboard);
    connect(&clearAction, &QAction::triggered, this,
            &CustomTableWidget::clearContent);

    contextMenu.addAction(&clearAction);
    contextMenu.addAction(&importAction);

    contextMenu.exec(event->globalPos());
}

void CustomTableWidget::importFromClipboard() {
    QClipboard *clipboard = QApplication::clipboard();
    QString clipboardText = clipboard->text();

    setRowCount(0);

    // Each line in clipboardText corresponds to a row in Excel
    QStringList rows = clipboardText.split('\n', Qt::SkipEmptyParts);

    // The row and column count of the data we are importing
    int rowCount = rows.size();
    int columnCount = rows.first().split('\t').size();

    // Check if column count matches
    if (this->columnCount() != columnCount) {
        QMessageBox::warning(
            this, "Import Error",
            "The imported data does not have the same number "
            "of columns as the table.");
        emit this->tableCleared();
        return;
    }


    // The starting row index for appending data
    int startRowIndex = this->rowCount();

    // Resize the table if necessary
    this->setRowCount(startRowIndex + rowCount);

    // Add the data to the table
    for (int i = 0; i < rowCount; ++i) {
        QStringList columns = rows.at(i).split('\t');
        for (int j = 0; j < columnCount; ++j) {
            bool ok;
            // Check if we have a delegate for this column
            NumericDelegate* delegate =
                qobject_cast<NumericDelegate*>(itemDelegateForColumn(j));
            IntNumericDelegate* intdelegate =
                qobject_cast<IntNumericDelegate*>(itemDelegateForColumn(j));
            CheckboxDelegate* checkboxDelegate =
                qobject_cast<CheckboxDelegate*>(itemDelegateForColumn(j));
            ComboBoxDelegate* comboDelegate =
                qobject_cast<ComboBoxDelegate*>(itemDelegateForColumn(j));
            NonEmptyDelegate* neDelegate =
                qobject_cast<NonEmptyDelegate*>(itemDelegateForColumn(j));


            if (delegate) {
                double val = columns.at(j).toDouble(&ok);
                if (ok) {
                    if (Utils::isValueInRange(val,
                                              delegate->getMinValue(),
                                              delegate->getMaxValue())) {
                        QTableWidgetItem* item =
                            new QTableWidgetItem(columns.at(j));
                        this->setItem(startRowIndex + i, j, item);
                    }
                }
                else {
                    QTableWidgetItem* item =
                        new QTableWidgetItem(delegate->getDefaultValue());
                    this->setItem(startRowIndex + i, j, item);
                }
            }
            else if (intdelegate) {
                int val = columns.at(j).toInt(&ok);
                if (ok) {
                    if (Utils::isValueInRange(val,
                                              intdelegate->getMinValue(),
                                              intdelegate->getMaxValue())) {
                        QTableWidgetItem* item =
                            new QTableWidgetItem(columns.at(j));
                        this->setItem(startRowIndex + i, j, item);
                    }
                }
                else {
                    QTableWidgetItem* item =
                        new QTableWidgetItem(intdelegate->getDefaultValue());
                    this->setItem(startRowIndex + i, j, item);
                }
            }
            else if (checkboxDelegate) {
                if (columns.at(j) == "1" || columns.at(j).toLower() == "true") {
                    QTableWidgetItem* item = new QTableWidgetItem();
                    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                    item->setCheckState(Qt::Checked);
                    this->setItem(startRowIndex + i, j, item);
                } else if (columns.at(j) == "0" ||
                           columns.at(j).toLower() == "false"){
                    QTableWidgetItem* item = new QTableWidgetItem();
                    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                    item->setCheckState(Qt::Unchecked);
                    this->setItem(startRowIndex + i, j, item);
                }
            }
            else if (comboDelegate) {
                QString result = Utils::findClosestMatch(columns.at(j),
                                                  comboDelegate->getValues());
                QTableWidgetItem* item =
                    new QTableWidgetItem(result);
                this->setItem(startRowIndex + i, j, item);
            }
            else if (neDelegate) {
                if (neDelegate->getFunctionality() == "ID") {
                    if (checkAllRowsHasThisValue(columns.at(j), j)) {
                        QString result = QString::number(generateUniqueID());
                        QTableWidgetItem* item =
                            new QTableWidgetItem(result);
                        this->setItem(startRowIndex + i, j, item);

                    }
                    else {
                        QTableWidgetItem* item =
                            new QTableWidgetItem(columns.at(j));
                        this->setItem(startRowIndex + i, j, item);
                    }
                }
                else {
                    QTableWidgetItem* item =
                        new QTableWidgetItem(columns.at(j));
                    this->setItem(startRowIndex + i, j, item);
                }

            }

            else {
                QTableWidgetItem* item =
                    new QTableWidgetItem(columns.at(j));
                this->setItem(startRowIndex + i, j, item);
            }
            //NonEmptyDelegate

        }
    }

}


void CustomTableWidget::clearContent()
{
    //int rowCount = this->rowCount();
    setRowCount(0);
    emit this->tableCleared();
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

bool CustomTableWidget::isRowEmpty(int row,
                                   const QList<int>& exceptionColumns) {
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



void CustomTableWidget::setCheckboxDelegateForColumns(
    int row, const QList<int> &columns) {
//    for (auto& column: columns) {
//        QCheckBox* checkbox = new QCheckBox(this);

//        int cellWidth = this->columnWidth(column);
//        int approxCheckboxWidth = 20; // an approximate width of the checkbox, this value might need adjusting
//        int margin = (cellWidth - approxCheckboxWidth) / 2;

//        checkbox->setStyleSheet(QString("margin-left: %1px; margin-right: %1px;").arg(margin));
//        setCellWidget(row, column, checkbox);
//    }
}

void CustomTableWidget::setNumericDelegateForColumns(
    int row, const QList<int> &columns) {

}


void CustomTableWidget::setupTable() {
//    for (int column = 0; column < columnCount(); column++) {
//        if (itemDelegateForColumn(column) != nullptr) {
//            QStyledItemDelegate* delegate = qobject_cast<QStyledItemDelegate*>(itemDelegateForColumn(column));
//            if (delegate != nullptr) {
//                if (qobject_cast<CheckboxDelegate*>(delegate)) {
//                    for (int row = 0; row < rowCount(); row++) {
//                        QTableWidgetItem* item = QTableWidget::item(row, column);
//                        if (item == nullptr) {
//                            setCheckboxDelegateForColumns(row, {column});
//                        }
//                    }
//                }
//            }
//        }
//    }
}

int CustomTableWidget::generateUniqueID() {
    // Find the maximum value in the first column
    int maxID = 0;
    if (rowCount() == 0) {
        return 1;
    }

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

void CustomTableWidget::setData(const QModelIndex& index,
                                const QVariant& value,
                                int role) {
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


//// Function to calculate the Levenshtein distance between two strings
//int CustomTableWidget::levenshteinDistance(const QString& str1,
//                                           const QString& str2) {
//    const int len1 = str1.length();
//    const int len2 = str2.length();

//    std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1, 0));

//    for (int i = 0; i <= len1; ++i)
//        dp[i][0] = i;

//    for (int j = 0; j <= len2; ++j)
//        dp[0][j] = j;

//    for (int i = 1; i <= len1; ++i) {
//        for (int j = 1; j <= len2; ++j) {
//            int cost = (str1[i - 1] == str2[j - 1]) ? 0 : 1;
//            dp[i][j] = std::min({ dp[i - 1][j] + 1,
//                                 dp[i][j - 1] + 1,
//                                 dp[i - 1][j - 1] + cost });
//        }
//    }

//    return dp[len1][len2];
//}

//// Function to find the closest match in the vector of QStrings
//QString CustomTableWidget::findClosestMatch(const QString& input,
//                         const std::vector<QString>& values) {
//    int minDistance = std::numeric_limits<int>::max();
//    QString closestMatch;

//    for (const QString& value : values) {
//        int distance = levenshteinDistance(value, input);
//        if (distance < minDistance) {
//            minDistance = distance;
//            closestMatch = value;
//        }
//    }

//    return closestMatch;
//}

bool CustomTableWidget::checkAllRowsHasThisValue(const QString value,
                                                 const int columnIndex) {
    for (int i = 0; i < rowCount(); i++) {
        item(i, columnIndex);
        QTableWidgetItem *item = this->item(i, columnIndex);
        if (item) {
            if (item->text() == value) {
                return true;
            }
        }
    }
    return false;
}

