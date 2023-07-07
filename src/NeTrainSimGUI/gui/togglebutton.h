#ifndef TOGGLEBUTTON_H
#define TOGGLEBUTTON_H

#include <QObject>
#include <QWidget>
#include <QPushButton>


class ToggleButton : public QPushButton {
    Q_OBJECT

public:
    explicit ToggleButton(QWidget *parent = nullptr);
    ~ToggleButton();

    void setOnOffText(const QString &onText, const QString &offText); // method to set on and off text
    bool isToggled() const;  // method to get if the button is on or off


private:
    QString onText;
    QString offText;
};

#endif // TOGGLEBUTTON_H
