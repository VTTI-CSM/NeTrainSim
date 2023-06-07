#ifndef NONEMPTYDELEGATE_H
#define NONEMPTYDELEGATE_H

#include "Netrainsim.h"
#include "qlineedit.h"
#include "qstyleditemdelegate.h"

class NonEmptyDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    NonEmptyDelegate(QObject* parent=nullptr) : QStyledItemDelegate(parent) {}

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
