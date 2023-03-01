#ifndef INTNUMERICDELEGATE_H
#define INTNUMERICDELEGATE_H

#include <QItemDelegate>
#include <QSpinBox>

class IntNumericDelegate : public QItemDelegate {
public:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QSpinBox *editor = new QSpinBox(parent);
        editor->setMinimum(-1000000000);
        editor->setMaximum(1000000000);
        return editor;
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        QSpinBox *spinBox = qobject_cast<QSpinBox *>(editor);
        int value = spinBox->value();
        model->setData(index, value, Qt::EditRole);
    }
};

#endif // INTNUMERICDELEGATE_H
