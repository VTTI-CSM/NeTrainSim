/**
 * @file NonEmptyDelegate.h
 * @brief This file contains the declaration of the NonEmptyDelegate class.
 *        The NonEmptyDelegate class is a QStyledItemDelegate subclass that provides functionality to ensure that table cells are not empty.
 *        It checks for empty cells and displays a warning message if a cell is empty.
 *        The NonEmptyDelegate class is intended to be used with QLineEdit-based editors in a table view.
 *        It is typically used with a QAbstractItemModel-based model.
 *        The NonEmptyDelegate class is derived from QStyledItemDelegate and utilizes QLineEdit as the editor widget.
 *        This class is suitable for handling non-empty cells in a table view with QLineEdit editors.
 *        When the user finishes editing a cell, the model data is checked for emptiness and the appropriate warning message is displayed.
 *        The NonEmptyDelegate class can be used in a QWidget-based application.
 *        Note: The NonEmptyDelegate class assumes that the parent widget is of type NeTrainSim, which provides the showWarning() function.
 * @author Ahmed Aredah
 * @date 6/7/2023
 */

#ifndef NONEMPTYDELEGATE_H
#define NONEMPTYDELEGATE_H

#include "netrainsimmainwindow.h"
#include <QLineEdit>
#include <QStyledItemDelegate>

/**
 * @class NonEmptyDelegate
 * @brief The NonEmptyDelegate class provides functionality to ensure that table cells are not empty.
 */
class NonEmptyDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    /**
     * @brief Constructs a NonEmptyDelegate object with the given parent.
     * @param parent The parent object.
     */
    NonEmptyDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    /**
     * @brief Sets the model data based on the data of the editor widget.
     * @param editor The editor widget.
     * @param model The abstract item model.
     * @param index The model index of the item.
     */
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
        if (lineEdit && lineEdit->text().trimmed().isEmpty()) {
            if (parent() != nullptr) {
                NeTrainSim* netP = qobject_cast<NeTrainSim*>(parent());
                netP->showWarning("The cell cannot be empty!");
            }
            return;
        }

        QString value = lineEdit->text().trimmed();
        bool isNumeric;
        value.toDouble(&isNumeric);

        if (!isNumeric) {
            if (parent() != nullptr) {
                NeTrainSim* netP = qobject_cast<NeTrainSim*>(parent());
                netP->showWarning("The value must be numeric!");
            }
            return;
        }

        QStyledItemDelegate::setModelData(editor, model, index);
    }
};

#endif // NONEMPTYDELEGATE_H
