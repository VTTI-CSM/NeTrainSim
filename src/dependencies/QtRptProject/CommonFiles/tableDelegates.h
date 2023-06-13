/***********************
*   Aleksey Osipov     *
*  aliks-os@ukr.net    *
*     2011-2015        *
************************/

#ifndef TABLEDELEGATES_H
#define TABLEDELEGATES_H

#include <QtGui>
#include <QTableWidgetItem>
#include <QItemDelegate>

//Обычный делегат, в котором есть сигнал о закрытии
class ItemDelegate : public QItemDelegate {
     Q_OBJECT
public:
    ItemDelegate(QObject *parent = 0);
     QWidget *createEditor(QWidget *parent,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private slots:
    void commitAndCloseEditor();
    void editorClose_(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);

signals:
    void editorClose(QItemDelegate *item);
};

//Описываем делегат с проверкой на числа
class DigDelegate : public QItemDelegate {
     Q_OBJECT
public:
    DigDelegate(QObject *parent = 0, QString type = "");
     QWidget *createEditor(QWidget *parent,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
    QString m_type;
private slots:
    void commitAndCloseEditor();
    void editorClose_(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);

signals:
    void editorClose(QItemDelegate *item);
};

//Описывем делегат-календарь
class CalendarDelegate : public QItemDelegate {
     Q_OBJECT
public:
     CalendarDelegate(bool calpopup = true, QObject *parent = 0);
     QWidget *createEditor(QWidget *parent,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
    bool m_calpopup;

private slots:
    void commitAndCloseEditor();
};

//Описывем делегат-ДБЛ редактор
class SpinDelegate : public QItemDelegate {
    Q_OBJECT
public:
    SpinDelegate(double min=0.00,
                 double max=999999999.99,
                 double step=0.1,
                 int precision=2,
                 QObject *parent = 0);
    QWidget *createEditor(
                QWidget *parent,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const;
    void updateEditorGeometry(
            QWidget *editor,
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const;
private:
    double m_min;
    double m_max;
    double m_step;
    int m_precision;

private slots:
    void commitAndCloseEditor();
};

//Описываем класс делегата-редактора с кнопкой
class BtnDelegate : public QItemDelegate {
    Q_OBJECT
public:
    BtnDelegate(QObject *parent = 0);
    QWidget *createEditor(
                QWidget *parent,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const;
    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const;
    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const;
    void updateEditorGeometry(
            QWidget *editor,
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const;

private slots:
    void commitAndCloseEditor();
    void editorClose_(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);
    void currentItemChanged_( QTableWidgetItem * current, QTableWidgetItem * previous);

signals:
    void clicked();
    void editorClose(QItemDelegate *item);
};

//Описываем делегат Combobox
typedef QList<QPair<QString, QVariant> > ComboValues;
class ComboboxDelegate : public QItemDelegate {
    Q_OBJECT
public:
    ComboboxDelegate(ComboValues comboValues, QObject *parent = 0);
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(
            QWidget *editor,
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const;
    QVariant getValue(QString key);

private:
    ComboValues m_comboValues;
};



#endif // TABLEDELEGATES_H
