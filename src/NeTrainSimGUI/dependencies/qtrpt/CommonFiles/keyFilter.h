#ifndef KEYFILTER_H
#define KEYFILTER_H

#include <QObject>
#include <QWidget>
#include <QtGui>

class KeyFilter : public QObject {
    Q_OBJECT
public:
    explicit KeyFilter(QWidget *parent = 0);
protected:
    bool eventFilter(QObject *obj, QEvent *event);

signals:
    void keyDelete();
    void keyEnter();
    void keyInsert();
};

#endif // KEYFILTER_H
