/**
 * @file NumericDelegate.h
 * @brief This file contains the declaration of the NumericDelegate class.
 *        The NumericDelegate class is a QItemDelegate subclass that
 *        provides a double spin box editor for numeric data in a view.
 *        It allows users to edit numeric values using double spin
 *         boxes in a model-based view.
 *        The NumericDelegate class sets the minimum and maximum values
 *         of the spin box to -1,000,000,000 and 1,000,000,000, respectively.
 *        It also sets the number of decimal places to 3.
 *        The NumericDelegate class is intended to be used with numeric
 *         data in a table or list view.
 *        It is typically used with a QAbstractItemModel-based model.
 *        The NumericDelegate class is derived from QItemDelegate and
 *        utilizes QDoubleSpinBox as the editor widget.
 *        This class is suitable for handling double numeric values
 *        in a table or list view with a QDoubleSpinBox editor.
 *        The NumericDelegate class can be used in a QWidget-based application.
 *        Note: The NumericDelegate class does not handle display
 *        text formatting.
 * @author Ahmed Aredah
 * @date 6/7/2023
 */

#ifndef NUMERICDELEGATE_H
#define NUMERICDELEGATE_H

#include <QItemDelegate>
#include <QDoubleSpinBox>

/**
 * @class NumericDelegate
 * @brief The NumericDelegate class provides a double spin box
 *        editor for numeric data in a view.
 */
class NumericDelegate : public QItemDelegate {
    Q_OBJECT

private:
    double maxValue;
    double minValue;
    int decimals;
    double stepSize;
    double value;
    QString defaultValueString;

    void setupSpinBox(QDoubleSpinBox *spinBox,
                      const QModelIndex &index) const {
        QVariant data = index.model()->data(index, Qt::EditRole);
        if (data.isValid() && data.canConvert<double>()) {
            spinBox->setValue(data.toDouble());
        } else {
            spinBox->setValue(value);
        }
    }

public:
    /**
     * @brief Constructor for the NumericDelegate class.
     * @param parent The parent widget.
     * @param maxValue The maximum value for the double spin box.
     * @param minValue The minimum value for the double spin box.
     * @param decimals The number of decimal places for the double spin box.
     * @param stepSize The step size for the double spin box.
     */
    NumericDelegate(QWidget *parent = nullptr,
                    double maxValue = 1000000000.0,
                    double minValue = -1000000000.0,
                    int decimals = 3,
                    double stepSize = 0.1,
                    double value = 0.0)
        : QItemDelegate(parent),
        maxValue(maxValue),
        minValue(minValue),
        decimals(decimals),
        stepSize(stepSize),
        value(value),
        defaultValueString(QString::number(value)) {}

    /**
     * @brief Creates an editor widget for the given parent,
     *        style option, and model index.
     * @param parent The parent widget.
     * @param option The style options for the item.
     * @param index The model index of the item.
     * @return The created editor widget.
     */
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override {
        QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
        editor->setMaximum(maxValue);
        editor->setMinimum(minValue);
        editor->setDecimals(decimals);
        editor->setSingleStep(stepSize);
        editor->setValue(value);
        // Set the default value if the cell doesn't contain valid data
        setupSpinBox(editor, index);
        return editor;
    }

    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override {
        QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *>(editor);
        if(spinBox){
            setupSpinBox(spinBox, index);
        }
    }

    /**
     * @brief Sets the model data based on the data of the editor widget.
     * @param editor The editor widget.
     * @param model The abstract item model.
     * @param index The model index of the item.
     */
    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override {
        QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *>(editor);
        if(spinBox){
            model->setData(index, spinBox->value(), Qt::EditRole);
        }
    }

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override {
        QStyleOptionViewItem opt = option;

        QVariant data = index.model()->data(index, Qt::EditRole);
        if (data.isValid() && data.canConvert<double>()) {
            opt.text = QString::number(data.toDouble());
            // Apply grey text color
            QPalette palette = opt.palette;
            palette.setColor(QPalette::Text, Qt::black);
            opt.palette = palette;
        }
        else {
             opt.text = "Ex: " + defaultValueString;
            // Apply grey text color
            QPalette palette = opt.palette;
            palette.setColor(QPalette::Text, Qt::gray);
            opt.palette = palette;
        }
//        opt.text = data.isValid() &&
//                           data.canConvert<int>()
//                       ? QString::number(data.toInt()) :
//                       "Ex: " + defaultValueString;

        QItemDelegate::paint(painter, opt, index);
    }

    void drawDisplay(QPainter *painter,
                     const QStyleOptionViewItem &option,
                     const QRect &rect,
                     const QString &text) const override {
        QItemDelegate::drawDisplay(painter, option, rect,
                                   text.isEmpty() ?
                                       "EX: " + defaultValueString : text);
    }

    double getMaxValue() {
        return maxValue;
    }

    double getMinValue() {
        return minValue;
    }

    double getDefaultValue() {
        return value;
    }
};

#endif // NUMERICDELEGATE_H
