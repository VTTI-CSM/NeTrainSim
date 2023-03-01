/**
 * @file	C:\Users\Ahmed\OneDrive - Virginia
 * 			Tech\03.Work\02.VTTI\02.ResearchWork\01.TrainModelling\02.Code\00.CPP\NeTrainSim\NeTrainSim\netrainsim.h.
 *
 * Declares the netrainsim class
 */
#ifndef NETRAINSIM_H
#define NETRAINSIM_H

#include "src/dependencies/qcustomplot/qcustomplot.h"
#include <QMainWindow>
#include "src/util/Map.h"
#include <iostream>

QT_BEGIN_NAMESPACE
namespace Ui { class NeTrainSim; }
QT_END_NAMESPACE

/**
 * A ne train simulation.
 *
 * @author	Ahmed Aredah
 * @date	2/28/2023
 */
class NeTrainSim : public QMainWindow
{
    Q_OBJECT

public:

    /**
     * Constructor
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param [in,out]	parent	(Optional) If non-null, the parent.
     */
    NeTrainSim(QWidget *parent = nullptr);

    /**
     * Destructor
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    ~NeTrainSim();

private:
    /** The user interface */
    Ui::NeTrainSim *ui;
    /** Define a vector to hold the labels */
    QVector<QCPItemText*> labelsVector;
    /** Define a vector to hold the nodes */
    Map<QString, std::pair<double, double>> networkNodes;

    /**
     * Sets up the generals
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    void setupGenerals();

    /**
     * Sets up the page 0
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    void setupPage0();

    /**
     * Sets up the page 1
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    void setupPage1();

    /**
     * Sets up the page 2
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    void setupPage2();

    /**
     * Sets up the page 3
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    void setupPage3();

    /**
     * Sets up the page 4
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    void setupPage4();
};
#endif // NETRAINSIM_H
