#include "disappearinglabel.h"


DisappearingLabel::DisappearingLabel(QWidget *parent) :
    QLabel(parent)
{
    // Connect timer to clearText slot
    connect(&m_timer, &QTimer::timeout, this, &DisappearingLabel::clearText);
}

void DisappearingLabel::setTextWithTimeout(const QString &text, int timeoutMs)
{
    // Set the label text and show it
    setText(text);
    show();

    // Start the timer to clear the label text after the specified timeout
    m_timer.start(timeoutMs);
}

void DisappearingLabel::clearText()
{
    // Clear the label text and hide it
    clear();
    hide();

    // Stop the timer
    m_timer.stop();
}
