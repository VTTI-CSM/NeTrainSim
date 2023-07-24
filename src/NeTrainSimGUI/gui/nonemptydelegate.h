/**
 * @file NonEmptyDelegate.h
 * @brief This file contains the declaration of the NonEmptyDelegate class.
 *        The NonEmptyDelegate class is a QStyledItemDelegate subclass that
 *         provides functionality to ensure that table cells are not empty.
 *        It checks for empty cells and displays a warning message
 *         if a cell is empty.
 *        The NonEmptyDelegate class is intended to be used with
 *        QLineEdit-based editors in a table view.
 *        It is typically used with a QAbstractItemModel-based model.
 *        The NonEmptyDelegate class is derived from QStyledItemDelegate
 *        and utilizes QLineEdit as the editor widget.
 *        This class is suitable for handling non-empty cells
 *        in a table view with QLineEdit editors.
 *        When the user finishes editing a cell, the model data is checked
 *        for emptiness and the appropriate warning message is displayed.
 *        The NonEmptyDelegate class can be used in a QWidget-based application.
 *        Note: The NonEmptyDelegate class assumes that the parent
 *        widget is of type NeTrainSim, which provides the showWarning()
 *        function.
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
 * @brief The NonEmptyDelegate class provides functionality
 * to ensure that table cells are not empty.
 */
class NonEmptyDelegate : public QStyledItemDelegate {
    Q_OBJECT

private:
    QString m_usedFor;
    QString m_defaultValue;

public:
    /**
     * @brief Constructs a NonEmptyDelegate object with the given parent.
     * @param parent The parent object.
     */
    NonEmptyDelegate(QString usedFor, QObject* parent = nullptr,
                     QString defaultValue = "") :
                     QStyledItemDelegate(parent),
                     m_usedFor(usedFor),
                     m_defaultValue(defaultValue) {}

    /**
     * @brief Sets the model data based on the data of the editor widget.
     * @param editor The editor widget.
     * @param model The abstract item model.
     * @param index The model index of the item.
     */
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override {
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
        double theValue = value.toDouble(&isNumeric);

        if (!isNumeric) {
            if (parent() != nullptr) {
                NeTrainSim* netP = qobject_cast<NeTrainSim*>(parent());
                netP->showWarning("The value must be numeric!");
            }
            return;
        }
        if (m_usedFor.toLower() == "id") {
            if (theValue < 0) {
                if (parent() != nullptr) {
                    NeTrainSim* netP = qobject_cast<NeTrainSim*>(parent());
                    netP->showWarning("ID value must be greater than 0!");
                }
                return;
            }
        }

        QStyledItemDelegate::setModelData(editor, model, index);
    }

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override {
        QStyleOptionViewItem opt = option;

        QVariant data = index.model()->data(index, Qt::EditRole);
        if (data.isValid()) {
            opt.text = data.toString();
            // Apply grey text color
            QPalette palette = opt.palette;
            palette.setColor(QPalette::Text, Qt::black);
            opt.palette = palette;
        }
        else {
            if (!m_defaultValue.isEmpty())
            {
                opt.text = "Ex: " + m_defaultValue;
                // Apply grey text color
                QPalette palette = opt.palette;
                palette.setColor(QPalette::Text, Qt::gray);
                opt.palette = palette;
            }
        }
        //        opt.text = data.isValid() &&
        //                           data.canConvert<int>()
        //                       ? QString::number(data.toInt()) :
        //                       "Ex: " + defaultValueString;

        QStyledItemDelegate::paint(painter, opt, index);
    }

    QString getFunctionality() {
        return m_usedFor;
    }
};

#endif // NONEMPTYDELEGATE_H
