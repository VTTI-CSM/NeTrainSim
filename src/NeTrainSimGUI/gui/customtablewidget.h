/**
 * @file CustomTableWidget.h
 * @brief This file contains the declaration of the CustomTableWidget class.
 *        The CustomTableWidget class is a subclass of QTableWidget that
 *        provides additional functionality and customization for table widgets.
 *        It includes methods for checking empty cells, setting
 *        delegates for specific columns, setting up the table, and
 *        generating unique IDs.
 *        The CustomTableWidget class also emits signals for key press
 *        events and when row deletion is not allowed.
 *        It overrides the keyPressEvent() method and the setData()
 *        method for custom handling.
 *        The CustomTableWidget class is intended to be used in
 *        a QWidget-based application.
 * @author Ahmed Aredah
 * @date 6/7/2023
 */

#ifndef CUSTOMTABLEWIDGET_H
#define CUSTOMTABLEWIDGET_H

#include <QTableWidget>
#include <QKeyEvent>
#include <QSet>

/**
 * @class CustomTableWidget
 * @brief The CustomTableWidget class is a subclass of QTableWidget
 * that provides additional functionality and customization for table widgets.
 */
class CustomTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a CustomTableWidget object.
     * @param parent The parent widget.
     */
    CustomTableWidget(QWidget *parent = nullptr);

    /**
     * @brief Checks if the table has any empty cells.
     * @param exceptionColumns The columns to exclude from the check.
     * @return True if there is at least one empty cell, false otherwise.
     */
    bool hasEmptyCell(const QList<int>& exceptionColumns = QList<int>());

    /**
     * @brief Checks if a row is empty.
     * @param row The row index.
     * @param exceptionColumns The columns to exclude from the check.
     * @return True if the row is empty, false otherwise.
     */
    bool isRowEmpty(int row,
                    const QList<int>& exceptionColumns = QList<int>());

    /**
     * Checks if the table is incomplete, i.e., if it does not have
     * at least one complete row of data.
     * A row is considered complete if all cells in the row have
     * non-empty data.
     *
     * @param exceptionColumns (optional) A list of column indices to
     * exclude from the completeness check.
     *                          Any rows that only have empty cells in
     *                          the specified columns will still be
     *                          considered incomplete.
     *                          Defaults to an empty list, which means
     *                          all columns will be considered for completeness.
     *
     * @return true if the table is incomplete, i.e., it does not have
     * at least one complete row of data. false if the table has at
     * least one complete row.
     */
    bool isTableIncomplete(const QList<int>& exceptionColumns = QList<int>());

    /**
     * @brief Sets a checkbox delegate for the specified columns in a row.
     * @param row The row index.
     * @param columns The columns to set the checkbox delegate for.
     */
    void setCheckboxDelegateForColumns(int row, const QList<int>& columns);

    /**
     * @brief Sets a numeric delegate for the specified columns in a row.
     * @param row The row index.
     * @param columns The columns to set the numeric delegate for.
     */
    void setNumericDelegateForColumns(int row, const QList<int> &columns);

    /**
     * @brief Performs initial setup for the table.
     *        This includes setting up default properties and signals/slots
     *        connections.
     */
    void setupTable();

    /**
     * @brief Generates a unique ID.
     *        This method can be used to generate unique identifiers for
     *        table entries.
     * @return The generated unique ID.
     */
    int generateUniqueID();

    void clearContent();

signals:
    /**
     * @brief Signal emitted when a key press event occurs in the table.
     * @param event The key event.
     */
    void keyPress(QKeyEvent *event);

    /**
     * @brief Signal emitted when row deletion is not allowed.
     *        This signal indicates that a row cannot be deleted.
     */
    void cannotDeleteRow();

    /**
     * @brief Signal emitted when the table content is
     * clear and all test is removed.
     */
    void tableCleared();

private slots:
    /**
     * @brief Deletes the selected rows from the table.
     */
    void deleteSelectedRows();

protected:
    /**
     * @brief Overridden keyPressEvent() method to handle key press events
     * in the table.
     * @param event The key event.
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief Overridden setData() method for custom handling of data
     * changes in the table.
     * @param index The model index of the item.
     * @param value The new value for the item.
     * @param role The role for the item.
     */
    void setData(const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole);

    void contextMenuEvent(QContextMenuEvent *event) override;

public slots:
    void importFromClipboard();

private:
    int levenshteinDistance(const QString& str1,
                            const QString& str2);
    QString findClosestMatch(const QString& input,
                             const std::vector<QString>& values);

    bool checkAllRowsHasThisValue(const QString value, const int columnIndex);
};

#endif // CUSTOMTABLEWIDGET_H
