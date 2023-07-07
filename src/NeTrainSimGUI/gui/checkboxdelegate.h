#ifndef CHECKBOXDELEGATE_H
#define CHECKBOXDELEGATE_H

#include "qapplication.h"
#include "qpainter.h"
#include <QStyledItemDelegate>
#include <QCheckBox>

class CheckboxDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit CheckboxDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent) {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override {
        if (index.data().canConvert<bool>()) {
            bool checked = index.data().toBool();

            if (option.state & QStyle::State_Selected)
                painter->fillRect(option.rect, option.palette.highlight());

            QStyleOptionButton checkboxStyleOption;
            checkboxStyleOption.state = QStyle::State_Enabled | (checked ? QStyle::State_On : QStyle::State_Off);
            checkboxStyleOption.rect = checkBoxRect(option);

            QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkboxStyleOption, painter);
        } else {
            QStyledItemDelegate::paint(painter, option, index);
        }
    }

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override {
        QCheckBox *editor = new QCheckBox(parent);
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        QCheckBox *checkBox = qobject_cast<QCheckBox *>(editor);
        bool checked = index.data().toBool();
        checkBox->setChecked(checked);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        QCheckBox *checkBox = qobject_cast<QCheckBox *>(editor);
        bool checked = checkBox->isChecked();
        model->setData(index, checked);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        editor->setGeometry(checkBoxRect(option));
    }

private:
    QRect checkBoxRect(const QStyleOptionViewItem &option) const {
        QStyleOptionButton checkboxStyleOption;
        QRect checkboxRect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxStyleOption);
        QPoint checkboxPoint(option.rect.x() + option.rect.width() / 2 - checkboxRect.width() / 2,
                             option.rect.y() + option.rect.height() / 2 - checkboxRect.height() / 2);
        return QRect(checkboxPoint, checkboxRect.size());
    }
};

#endif // CHECKBOXDELEGATE_H
