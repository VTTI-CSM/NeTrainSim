#include "keyFilter.h"

KeyFilter::KeyFilter(QWidget *parent) {
    this->setParent(parent);
}

bool KeyFilter::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Delete) {
            emit keyDelete();
        }
        if (keyEvent->key() == Qt::Key_Return) {
            emit keyEnter();
        }
        if (keyEvent->key() == Qt::Key_Enter) {
            emit keyEnter();
        }
        if (keyEvent->key() == Qt::Key_Insert) {
            emit keyInsert();
        }
        //return true;
        // standard event processing
        return QObject::eventFilter(obj, event);
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
