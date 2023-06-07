#ifndef NUMERICDELEGATE_H
#define NUMERICDELEGATE_H


#include <QItemDelegate>
#include <QDoubleSpinBox>

class NumericDelegate : public QItemDelegate {
    Q_OBJECT
public:

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
        editor->setMinimum(-1000000000);
        editor->setMaximum(1000000000);
        editor->setDecimals(3);
        return editor;
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *>(editor);
        spinBox->interpretText();
        double value = spinBox->value();
        model->setData(index, value, Qt::EditRole);
    }
};

#endif // NUMERICDELEGATE_H
