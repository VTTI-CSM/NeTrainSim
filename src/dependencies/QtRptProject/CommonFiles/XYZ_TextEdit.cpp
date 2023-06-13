#include "XYZ_TextEdit.h"
#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QtDebug>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>

XYZTextEdit::XYZTextEdit(QWidget *parent)
: QTextEdit(parent), c(nullptr)
{
}

XYZTextEdit::~XYZTextEdit()
{
}

void XYZTextEdit::setCompleter(QCompleter *completer)
{
    if (c)
        QObject::disconnect(c, nullptr, this, nullptr);

    c = completer;

    if (!c)
        return;

    c->setWidget(this);
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(c, SIGNAL(activated(QString)),
                     this, SLOT(insertCompletion(QString)));
}

QCompleter *XYZTextEdit::completer() const
{
    return c;
}

void XYZTextEdit::insertCompletion(const QString& completion)
{
    if (c->widget() != this)
        return;

//    QTextCursor tc = textCursor();
//    int extra = completion.length() - c->completionPrefix().length();
//    tc.movePosition(QTextCursor::Left);
//    tc.movePosition(QTextCursor::EndOfWord);
//    tc.insertText(completion.right(extra));
//    setTextCursor(tc);

    QTextCursor tc = textCursor();
    int extra = completion.length() - c->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);

    textCursor().beginEditBlock();
    auto doc = document();
    auto cursor = QTextCursor(doc);
    while (true) {
        cursor = doc->find(completion, cursor/*, flags*/);
        if (cursor.isNull())
            break;
        cursor.insertText(completion);
    }
    textCursor().endEditBlock();
}

QString XYZTextEdit::textUnderCursor() const
{
//    QTextCursor tc = textCursor();
//    tc.select(QTextCursor::WordUnderCursor);
//    return tc.selectedText();

    QTextCursor tc = textCursor();
    bool isStartOfWord = false;
    if (tc.atStart() || tc.positionInBlock() == 0)
        isStartOfWord = true;
    while (!isStartOfWord) {
        tc.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
        if (tc.atStart() || tc.positionInBlock() == 0)
            isStartOfWord = true;
        else if (tc.selectedText().at(0) == QString("["))
            isStartOfWord = true;
    }
    return tc.selectedText().trimmed().replace("[", "");
}

void XYZTextEdit::focusInEvent(QFocusEvent *e)
{
    if (c)
        c->setWidget(this);
    QTextEdit::focusInEvent(e);
}

void XYZTextEdit::keyPressEvent(QKeyEvent *e)
{
    if (c && c->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
    if (!c || !isShortcut) // do not process the shortcut when we have a completer
        QTextEdit::keyPressEvent(e);

    //qDebug() << e->key() << isShortcut << e->modifiers() << e->text();

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!c || (ctrlOrShift && e->text().isEmpty()))
        return;

    //static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    static QString eow("~!@#$%^&*()_+{}|:\"<>?,/;']\\-="); // end of word
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    qDebug() << completionPrefix;

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 3
                      || eow.contains(e->text().right(1)))) {
        c->popup()->hide();
        return;
    }

    //completionPrefix = completionPrefix.replace("[", "");
    if (completionPrefix != c->completionPrefix()) {
        c->setCompletionPrefix(completionPrefix);
        c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(c->popup()->sizeHintForColumn(0)
                + c->popup()->verticalScrollBar()->sizeHint().width());
    c->complete(cr); // popup it up!
}

