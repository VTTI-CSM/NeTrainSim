#ifndef CUSTOMPROGRESSBAR_H
#define CUSTOMPROGRESSBAR_H

#include <QProgressBar>
#include <QTimer>

class CustomProgressBar : public QProgressBar {
    Q_OBJECT
public:
    CustomProgressBar(QWidget* parent = nullptr) : QProgressBar(parent) {
        setMinimum(0);
        setMaximum(100);
        setTextVisible(false);
        hide();
        timer_ = new QTimer(this);
        timer_->setSingleShot(true);
        connect(timer_, &QTimer::timeout, this, &CustomProgressBar::hide);
    }

    void start() {
        show();
        timer_->stop();
        emit progressStarted();
    }

    void stop() {
        timer_->start(5000);
        emit progressStopped();
    }

signals:
    void progressStarted();
    void progressStopped();

private:
    QTimer* timer_;
};



#endif // CUSTOMPROGRESSBAR_H
