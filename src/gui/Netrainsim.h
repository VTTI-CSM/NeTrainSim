/**
 * @file	~\NeTrainSim\netrainsim.h.
 *
 * Declares the netrainsim class
 */
#ifndef NETRAINSIM_H
#define NETRAINSIM_H

#include "qtrpt.h"
#include "src/gui/aboutwindow.h"
#include "src/gui/customplot.h"
//#include "src/dependencies/qcustomplot/qcustomplot.h"
#include <QMainWindow>
#include "src/util/Map.h"
#include <iostream>
#include "simulationworker.h"

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

signals:
    void nodesDataChanged(QVector<double>& xData, QVector<double>& yData, QVector<QString>& labels);
    void linksDataChanged(QVector<QString>& startNodeID, QVector<QString> endNodeID);

public slots:
    void setNodesData(QVector<double>& xData, QVector<double>& yData, QVector<QString>& labels);
    void setLinksData(QVector<QString>& startNodeIDs, QVector<QString> endNodeIDs);
    void updateTrainsPlot(Vector<std::pair<std::string, Vector<std::pair<double,double>>>> trainsStartEndPoints);

private slots:
    void showReport();
    void setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage);
    void setDSInfo(DataSetInfo &dsInfo);
    void handleError(std::string error);
    void handleSimulationFinished(Vector<std::pair<string, string> > summaryData);

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

    SimulationWorker* worker = nullptr;
    QThread* thread = nullptr;
    /** Define a vector to hold the labels */
    QVector<QCPItemText*> labelsVector;
    /** Define a vector to hold the nodes */
    Map<QString, std::pair<double, double>> networkNodes;

    QVector<double> nodesXData;
    QVector<double> nodesYData;
    QVector<QString> nodesLabelData;
    QVector<QString> linksStartNodeIDs;
    QVector<QString> linksEndNodeIDs;

    std::shared_ptr<AboutWindow> aboutWindow = nullptr;

    QString projectName, author, networkName, nodesFilename, linksFilename, trainsFilename, projectFileName;

    Vector<std::pair<std::string, std::string>> trainsSummaryData;


    void showDetailedReport(QString trajectoryFilename);

    void drawLineGraph(CustomPlot &plot, const QVector<double> &xData, const QVector<double> &yData, QString xLabel, QString yLabel, QString graphName, int plotIndex);



    void saveProjectFile(bool saveAs = false);

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

    Vector<std::tuple<int, double, double, std::string, double, double> > getNodesDataFromNodesTable();

    std::tuple<QVector<double>, QVector<double>,
          QVector<QString>> getNodesPlottableData(Vector<std::tuple<int, double, double, std::string,
                                                                         double, double>> &nodeRecords);

    // create a slot to update the QCustomPlot data and redraw the plot
    void updateNodesPlot(CustomPlot &plot, QVector<double>xData,
                                     QVector<double>yData, QVector<QString> labels, bool showLabels = false);

    Vector<std::tuple<int, int, int, double, double,
                      int, double, double, int, double,
                      bool, std::string, double, double> > getLinkesDataFromLinksFile(QString fileName);

    Vector<std::tuple<int, int, int, double, double,
                      int, double, double, int, double,
                      bool, std::string, double, double> > getLinkesDataFromLinksTable();

    std::tuple<QVector<QString>,
               QVector<QString>> getLinksPlottableData(Vector<std::tuple<int, int, int,
                                                        double, double, int,
                                                        double, double, int,
                                                        double, bool, std::string,
                                                        double, double>> linksRecords);

    void updateLinksPlot(CustomPlot &plot, QVector<QString> startNodeIDs, QVector<QString> endNodeIDs);

    QString browseFiles(QLineEdit* theLineEdit, const QString& theFileName);

    Vector<std::tuple<int, double, double, std::string,
                      double, double>> getNodesDataFromNodesFile(QString fileName);

    void browseFolder(QLineEdit* theLineEdit, const QString& theHelpMessage);


    Vector<std::tuple<std::string, Vector<int>, double, double,
                 Vector<std::tuple<double, double, double, double, double, double, int, int>>,
                      Vector<std::tuple<double, double, double, double, double, int, int>>,
                      bool>> getTrainsDataFromTables();

    void simulate();

public:
    void showNotification(QString text);
    void showWarning(QString text);

protected:
    void closeEvent(QCloseEvent* event) override;
};
#endif // NETRAINSIM_H
