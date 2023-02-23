#ifndef NETRAINSIM_H
#define NETRAINSIM_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class NeTrainSim; }
QT_END_NAMESPACE

class NeTrainSim : public QMainWindow
{
    Q_OBJECT

public:
    NeTrainSim(QWidget *parent = nullptr);
    ~NeTrainSim();

private:
    Ui::NeTrainSim *ui;
};
#endif // NETRAINSIM_H
