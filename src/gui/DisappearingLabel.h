/**
 * @file DisappearingLabel.h
 * @brief This file contains the declaration of the DisappearingLabel class.
 *        The DisappearingLabel class is a subclass of QLabel that provides functionality to display text for a specified duration and then clear it.
 *        It uses a QTimer to automatically clear the text after the specified timeout.
 *        The DisappearingLabel class is intended to be used in a QWidget-based application.
 *        The setTextWithTimeout() method can be used to set the text and timeout duration.
 *        The clearText() slot is called when the timeout occurs to clear the text.
 * @author Ahmed Aredah
 * @date 6/7/2023
 */

#ifndef DISAPPEARINGLABEL_H
#define DISAPPEARINGLABEL_H

#include <QLabel>
#include <QTimer>

/**
 * @class DisappearingLabel
 * @brief The DisappearingLabel class is a subclass of QLabel that provides functionality to display text for a specified duration and then clear it.
 */
class DisappearingLabel : public QLabel
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a DisappearingLabel object.
     * @param parent The parent widget.
     */
    explicit DisappearingLabel(QWidget *parent = nullptr);

    /**
     * @brief Sets the text with a timeout duration.
     *        The text will be displayed for the specified duration in milliseconds and then cleared.
     * @param text The text to display.
     * @param timeoutMs The timeout duration in milliseconds.
     */
    void setTextWithTimeout(const QString &text, int timeoutMs);

private:
    QTimer m_timer; /**< Timer used to clear the text after the timeout duration. */

private slots:
    /**
     * @brief Slot called when the timeout occurs to clear the text.
     */
    void clearText();
};

#endif // DISAPPEARINGLABEL_H
