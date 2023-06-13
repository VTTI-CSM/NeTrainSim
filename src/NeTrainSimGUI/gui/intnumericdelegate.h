/**
 * @file IntNumericDelegate.h
 * @brief This file contains the declaration of the IntNumericDelegate class.
 *        The IntNumericDelegate class is a QItemDelegate subclass that provides a spin box editor for integer numeric data in a view.
 *        It allows users to edit integer values using spin boxes in a model-based view.
 *        The IntNumericDelegate class sets the minimum and maximum values of the spin box to -1,000,000,000 and 1,000,000,000, respectively.
 *        The IntNumericDelegate class is intended to be used with integer numeric data in a table or list view.
 *        It is typically used with a QAbstractItemModel-based model.
 *        The IntNumericDelegate class is derived from QItemDelegate and utilizes QSpinBox as the editor widget.
 *        This class is suitable for handling integer numeric values in a table or list view with a QSpinBox editor.
 *        When the user modifies the spin box value, the model data is updated accordingly.
 *        The IntNumericDelegate class can be used in a QWidget-based application.
 *        Note: The IntNumericDelegate class does not handle display text formatting.
 * @author Ahmed Aredah
 * @date 6/7/2023
 */

#ifndef INTNUMERICDELEGATE_H
#define INTNUMERICDELEGATE_H

#include <QItemDelegate>
#include <QSpinBox>

/**
 * @class IntNumericDelegate
 * @brief The IntNumericDelegate class provides a spin box editor for integer numeric data in a view.
 */
class IntNumericDelegate : public QItemDelegate {
    Q_OBJECT

private:
    int maxValue;
    int minValue;
    int stepSize;
    int value;

public:
    /**
     * @brief Constructor for the IntNumericDelegate class.
     * @param parent The parent widget.
     * @param maxValue The maximum value for the spin box.
     * @param minValue The minimum value for the spin box.
     * @param stepSize The step size for the spin box.
     */
    IntNumericDelegate(QWidget *parent = nullptr, int maxValue = 1000000000, int minValue = -1000000000, int stepSize = 1, int value = 0)
        : QItemDelegate(parent), maxValue(maxValue), minValue(minValue), stepSize(stepSize), value(value) {}

    /**
     * @brief Creates an editor widget for the given parent, style option, and model index.
     * @param parent The parent widget.
     * @param option The style options for the item.
     * @param index The model index of the item.
     * @return The created editor widget.
     */
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QSpinBox *editor = new QSpinBox(parent);
        editor->setMaximum(maxValue);
        editor->setMinimum(minValue);
        editor->setSingleStep(stepSize);
        editor->setValue(value);
        return editor;
    }

    /**
     * @brief Sets the model data based on the data of the editor widget.
     * @param editor The editor widget.
     * @param model The abstract item model.
     * @param index The model index of the item.
     */
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        QSpinBox *spinBox = qobject_cast<QSpinBox *>(editor);
        int value = spinBox->value();
        model->setData(index, value, Qt::EditRole);
    }
};

#endif // INTNUMERICDELEGATE_H
