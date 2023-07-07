#include "togglebutton.h"
#include <QVBoxLayout>

ToggleButton::ToggleButton(QWidget *parent)
    : QPushButton(parent),
    onText("On"),
    offText("Off")
{
    setCheckable(true);
    setText(offText);

    connect(this, &QPushButton::toggled, this, [this](bool checked) {
        setText(checked ? onText : offText);
    });
}

ToggleButton::~ToggleButton() {
}

void ToggleButton::setOnOffText(const QString &onTxt, const QString &offTxt) {
    onText = onTxt;
    offText = offTxt;
    setText(isChecked() ? onText : offText);
}

bool ToggleButton::isToggled() const {
    return isChecked();
}
