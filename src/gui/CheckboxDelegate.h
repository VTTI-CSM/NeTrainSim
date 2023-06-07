/**
 * @file CheckboxDelegate.h
 * @brief This file contains the declaration of the CheckboxDelegate class.
 *        The CheckboxDelegate class is a QStyledItemDelegate subclass that provides a checkbox editor for boolean data in a view.
 *        It allows users to edit boolean values using checkboxes in a model-based view.
 *        This delegate does not display any text as it only shows checkboxes.
 * @author Ahmed Aredah
 * @date 6/7/2023
 */

#ifndef CHECKBOXDELEGATE_H
#define CHECKBOXDELEGATE_H

#include <QStyledItemDelegate>
#include <QCheckBox>

/**
 * @class CheckboxDelegate
 * @brief The CheckboxDelegate class provides a checkbox editor for boolean data in a view.
 *        It allows users to edit boolean values using checkboxes in a model-based view.
 *        This delegate does not display any text as it only shows checkboxes.
 */
class CheckboxDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    /**
     * @brief Returns the display text for the given value and locale.
     *        This delegate does not display any text, so an empty string is returned.
     * @param value The value to be displayed.
     * @param locale The locale used for formatting.
     * @return The display text.
     */
    QString displayText(const QVariant &value, const QLocale &locale) const override {
        Q_UNUSED(value);
        Q_UNUSED(locale);
        return QString();
    }

    /**
     * @brief Creates an editor widget for the given parent, style option, and model index.
     * @param parent The parent widget.
     * @param option The style options for the item.
     * @param index The model index of the item.
     * @return The created editor widget.
     */
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QCheckBox *editor = new QCheckBox(parent);
        return editor;
    }

    /**
     * @brief Sets the data of the editor widget based on the data in the model index.
     * @param editor The editor widget.
     * @param index The model index of the item.
     */
    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        QCheckBox *checkBox = qobject_cast<QCheckBox *>(editor);
        bool checked = index.model()->data(index, Qt::EditRole).toBool();
        checkBox->setChecked(checked);
    }

    /**
     * @brief Sets the data in the model index based on the data of the editor widget.
     * @param editor The editor widget.
     * @param model The abstract item model.
     * @param index The model index of the item.
     */
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        QCheckBox *checkBox = qobject_cast<QCheckBox *>(editor);
        bool checked = checkBox->isChecked();
        model->setData(index, checked, Qt::EditRole);
    }

    /**
     * @brief Updates the editor geometry to match the style options and model index.
     * @param editor The editor widget.
     * @param option The style options for the item.
     * @param index The model index of the item.
     */
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        editor->setGeometry(option.rect);
    }

};

#endif // CHECKBOXDELEGATE_H
