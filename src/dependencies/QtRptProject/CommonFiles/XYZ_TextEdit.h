#ifndef XYZ_TEXTEDIT_H
#define XYZ_TEXTEDIT_H


#include <QTextEdit>

QT_BEGIN_NAMESPACE
class QCompleter;
QT_END_NAMESPACE

class XYZTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    XYZTextEdit(QWidget *parent = nullptr);
    ~XYZTextEdit();

    void setCompleter(QCompleter *c);
    QCompleter *completer() const;

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;

private slots:
    void insertCompletion(const QString &completion);

private:
    QString textUnderCursor() const;

private:
    QCompleter *c;
};

#endif // XYZ_TEXTEDIT_H
