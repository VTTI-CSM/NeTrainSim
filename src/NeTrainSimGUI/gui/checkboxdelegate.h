#ifndef CHECKBOXDELEGATE_H
#define CHECKBOXDELEGATE_H

#include <QApplication>
#include <QCheckBox>
#include <QPainter>
#include <QStyledItemDelegate>

/*!
    \class CheckboxDelegate
    \brief A delegate to render and edit checkboxes in a table view.

    This class handles the display, editing, and updating of checkbox states in a QTableView
    or QTableWidget using the `Qt::CheckStateRole`.
*/
class CheckboxDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    /*!
        \brief Constructor for CheckboxDelegate.
        \param parent The parent object, typically the table view or widget using this delegate.
    */
    explicit CheckboxDelegate(QObject* parent = nullptr)
        : QStyledItemDelegate(parent) {}

    /*!
        \brief Paints the checkbox in the cell.
        \param painter The QPainter used to draw the checkbox.
        \param option The style options for the item.
        \param index The QModelIndex of the item to be painted.
    */
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        if (index.data(Qt::CheckStateRole).isValid()) {
            // Determine checkbox state
            bool checked = index.data(Qt::CheckStateRole).toInt() == Qt::Checked;

            // Highlight selected rows
            if (option.state & QStyle::State_Selected)
                painter->fillRect(option.rect, option.palette.highlight());

            // Draw the checkbox
            QStyleOptionButton checkboxStyleOption;
            checkboxStyleOption.state = QStyle::State_Enabled | (checked ? QStyle::State_On : QStyle::State_Off);
            checkboxStyleOption.rect = checkBoxRect(option);

            QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkboxStyleOption, painter);
        } else {
            // Default painting for non-checkbox cells
            QStyledItemDelegate::paint(painter, option, index);
        }
    }

    /*!
        \brief Creates the editor for the checkbox cell.
        \param parent The parent widget.
        \param option The style options for the editor.
        \param index The QModelIndex of the cell to be edited.
        \return A pointer to the created editor.
    */
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const override {
        return new QCheckBox(parent);
    }

    /*!
        \brief Sets the checkbox state in the editor.
        \param editor The editor widget.
        \param index The QModelIndex of the cell being edited.
    */
    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        QCheckBox* checkBox = qobject_cast<QCheckBox*>(editor);
        if (checkBox) {
            // Retrieve checkbox state from the model
            bool checked = index.data(Qt::CheckStateRole).toInt() == Qt::Checked;
            checkBox->setChecked(checked);
        }
    }

    /*!
        \brief Updates the model with the checkbox state from the editor.
        \param editor The editor widget.
        \param model The model being updated.
        \param index The QModelIndex of the cell being edited.
    */
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
        QCheckBox* checkBox = qobject_cast<QCheckBox*>(editor);
        if (checkBox) {
            // Update the model with the checkbox state
            model->setData(index, checkBox->isChecked() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
        }
    }

    /*!
        \brief Adjusts the geometry of the editor to fit the cell.
        \param editor The editor widget.
        \param option The style options for the editor.
        \param index The QModelIndex of the cell being edited.
    */
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const override {
        editor->setGeometry(checkBoxRect(option));
    }

private:
    /*!
        \brief Computes the rectangle for drawing the checkbox within the cell.
        \param option The style options for the cell.
        \return The rectangle for the checkbox.
    */
    QRect checkBoxRect(const QStyleOptionViewItem& option) const {
        QStyleOptionButton checkboxStyleOption;
        QRect checkboxRect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxStyleOption);
        QPoint checkboxPoint(option.rect.x() + option.rect.width() / 2 - checkboxRect.width() / 2,
                             option.rect.y() + option.rect.height() / 2 - checkboxRect.height() / 2);
        return QRect(checkboxPoint, checkboxRect.size());
    }
};

#endif // CHECKBOXDELEGATE_H
