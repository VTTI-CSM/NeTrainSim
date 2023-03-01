#ifndef DISAPPEARINGLABEL_H
#define DISAPPEARINGLABEL_H

#include <QLabel>
#include <QLabel>
#include <QTimer>

class DisappearingLabel : public QLabel
{
    Q_OBJECT

public:
    explicit DisappearingLabel(QWidget *parent = nullptr);
    void setTextWithTimeout(const QString &text, int timeoutMs);

private:
    QTimer m_timer;

private slots:
    void clearText();

};

#endif // DISAPPEARINGLABEL_H
