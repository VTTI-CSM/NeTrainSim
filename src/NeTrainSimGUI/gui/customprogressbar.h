/**
 * @file CustomProgressBar.h
 * @brief This file contains the declaration of the CustomProgressBar class.
 *        The CustomProgressBar class is a subclass of QProgressBar that provides a customized progress bar with start and stop functionality.
 *        It emits signals when the progress starts and stops.
 *        The progress bar is initially hidden and starts when the start() method is called.
 *        After a specified timeout period, it automatically stops and emits the progressStopped() signal.
 *        The timeout period is set to 5000 milliseconds (5 seconds) by default.
 *        The progress bar does not display text and uses a minimum value of 0 and a maximum value of 100.
 *        The text visibility is disabled.
 *        The CustomProgressBar class is intended to be used in a QWidget-based application.
 *        The progressStarted() and progressStopped() signals can be connected to slots to perform actions when the progress starts and stops, respectively.
 *        The CustomProgressBar class is derived from QProgressBar and utilizes QTimer for timing functionality.
 * @author Ahmed Aredah
 * @date 6/7/2023
 */

#ifndef CUSTOMPROGRESSBAR_H
#define CUSTOMPROGRESSBAR_H

#include <QProgressBar>
#include <QTimer>

/**
 * @class CustomProgressBar
 * @brief The CustomProgressBar class provides a customized progress bar with start and stop functionality.
 *        It emits signals when the progress starts and stops.
 */
class CustomProgressBar : public QProgressBar {
    Q_OBJECT

public:
    /**
     * @brief Constructs a CustomProgressBar object.
     * @param parent The parent widget.
     */
    CustomProgressBar(QWidget* parent = nullptr) : QProgressBar(parent) {
        setMinimum(0);
        setMaximum(100);
        setTextVisible(false);
        hide();
        timer_ = new QTimer(this);
        timer_->setSingleShot(true);
        connect(timer_, &QTimer::timeout, this, &CustomProgressBar::hide);
    }

    /**
     * @brief Destroys the CustomProgressBar object.
     */
    ~CustomProgressBar() {
        delete timer_;
    }

    /**
     * @brief Starts the progress bar.
     *        The progress bar is shown and the progressStarted() signal is emitted.
     */
    void start() {
        show();
        timer_->stop();
        emit progressStarted();
    }

    /**
     * @brief Stops the progress bar.
     *        The progress bar is hidden and the progressStopped() signal is emitted.
     *        After a timeout period of 5000 milliseconds (5 seconds), the progress bar automatically stops and emits the progressStopped() signal.
     */
    void stop() {
        timer_->start(5000);
        emit progressStopped();
    }

signals:
    /**
     * @brief Signal emitted when the progress starts.
     *        This signal indicates that the progress bar has started and is being displayed.
     */
    void progressStarted();

    /**
     * @brief Signal emitted when the progress stops.
     *        This signal indicates that the progress bar has stopped and is no longer being displayed.
     */
    void progressStopped();

private:
    QTimer* timer_; /**< Timer used for automatically stopping the progress bar after a timeout period. */
};

#endif // CUSTOMPROGRESSBAR_H
