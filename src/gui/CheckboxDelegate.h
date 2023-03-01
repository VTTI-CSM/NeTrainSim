#ifndef CHECKBOXDELEGATE_H
#define CHECKBOXDELEGATE_H

#include <QStyledItemDelegate>
#include <QCheckBox>

class CheckboxDelegate : public QStyledItemDelegate {
public:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QCheckBox *editor = new QCheckBox(parent);
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        QCheckBox *checkBox = qobject_cast<QCheckBox *>(editor);
        bool checked = index.model()->data(index, Qt::EditRole).toBool();
        checkBox->setChecked(checked);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        QCheckBox *checkBox = qobject_cast<QCheckBox *>(editor);
        bool checked = checkBox->isChecked();
        model->setData(index, checked, Qt::EditRole);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        editor->setGeometry(option.rect);
    }
};

#endif // CHECKBOXDELEGATE_H
