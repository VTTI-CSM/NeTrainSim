#ifndef TEXTBOXDELEGATE_H
#define TEXTBOXDELEGATE_H

#include "qitemdelegate.h"
#include "qlineedit.h"

class TextBoxDelegate : public QItemDelegate {
    Q_OBJECT

private:
    QString m_defaultValue;

    void setupLineEdit(QLineEdit *lineEdit,
                      const QModelIndex &index) const {
        QVariant data = index.model()->data(index, Qt::EditRole);
        if (data.isValid()) {
            lineEdit->setText(data.toString());
        } else {
            lineEdit->setText(m_defaultValue);
        }
    }

public:
    /**
     * @brief Constructs a NonEmptyDelegate object with the given parent.
     * @param parent The parent object.
     */
    TextBoxDelegate(QObject* parent = nullptr, QString defaultValue = "") :
        QItemDelegate(parent), m_defaultValue(defaultValue) {}

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
        QLineEdit *editor = new QLineEdit(parent);
        setupLineEdit(editor, index);
        return editor;
    }

    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override {
        QLineEdit *lineEdit= qobject_cast<QLineEdit *>(editor);
        if(lineEdit){
            setupLineEdit(lineEdit, index);
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
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
        if(lineEdit){
            model->setData(index, lineEdit->text(), Qt::EditRole);
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
            opt.text = "Ex: " + m_defaultValue;
            // Apply grey text color
            QPalette palette = opt.palette;
            palette.setColor(QPalette::Text, Qt::gray);
            opt.palette = palette;
        }

        QItemDelegate::paint(painter, opt, index);
    }

    void drawDisplay(QPainter *painter,
                     const QStyleOptionViewItem &option,
                     const QRect &rect,
                     const QString &text) const override {
        QItemDelegate::drawDisplay(painter, option, rect,
                                   text.isEmpty() ?
                                       "EX: " + m_defaultValue : text);
    }

};
#endif // TEXTBOXDELEGATE_H
