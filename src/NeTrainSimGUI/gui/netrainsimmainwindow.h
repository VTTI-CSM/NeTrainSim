/**
 * @file	~\NeTrainSim\netrainsim.h.
 *
 * Declares the netrainsim class
 */
#ifndef NETRAINSIMMAINWINDOW_H
#define NETRAINSIMMAINWINDOW_H

// #include "QtRPT/qtrpt.h"
#include "qlineedit.h"
#include "gui/aboutwindow.h"
#include "gui/customplot.h"
#include <QMainWindow>
#include "../NeTrainSim/util/map.h"
#include <any>
//#include "qtrpt/QtRPT/qtrpt.h"
#include "simulationworker.h"
#include "util/configurationmanager.h"
#include "settingswindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class NeTrainSim; }
QT_END_NAMESPACE

/**
 * @brief       A network train simulation GUI.
 *
 * @details     This class represents the main window of the NeTrainSim application, which is used
 *              for simulating train networks. It provides functionality for setting up the simulation,
 *              managing network nodes and links, visualizing the network, running simulations, and generating reports.
 *
 *              The class emits signals for notifying changes in the nodes and links data, and provides slots
 *              for handling events and updating the GUI based on these signals. It also includes several private
 *              methods for setting up the application, handling user interactions, and performing internal tasks.
 *
 * @author      Ahmed Aredah
 * @date        6/7/2023
 */
class NeTrainSim : public QMainWindow
{
    Q_OBJECT

signals:
    /**
     * @brief           Signal emitted when the nodes data is changed.
     *
     * @param xData     The updated X-coordinate data of the nodes.
     * @param yData     The updated Y-coordinate data of the nodes.
     * @param labels    The updated labels of the nodes.
     *
     * @author          Ahmed Aredah
     * @date            6/7/2023
     */
    void nodesDataChanged(QVector<double>& xData, QVector<double>& yData, QVector<QString>& labels);

    /**
     * @brief               Signal emitted when the links data is changed.
     *
     * @param startNodeIDs  The updated start node IDs of the links.
     * @param endNodeIDs    The updated end node IDs of the links.
     * @author              Ahmed Aredah
     * @date                6/7/2023
     */
    void linksDataChanged(QVector<QString>& startNodeID, QVector<QString> endNodeID);

public slots:
    /**
     * @brief           Slot for setting the nodes data.
     *
     * @param xData     The X-coordinate data of the nodes.
     * @param yData     The Y-coordinate data of the nodes.
     * @param labels    The labels of the nodes.
     *
     * @author          Ahmed Aredah
     * @date            6/7/2023
     */
    void setNodesData(QVector<double>& xData, QVector<double>& yData, QVector<QString>& labels);

    /**
     * @brief               Slot for setting the links data.
     *
     * @param startNodeIDs  The start node IDs of the links.
     * @param endNodeIDs    The end node IDs of the links.
     *
     * @author              Ahmed Aredah
     * @date                6/7/2023
     */
    void setLinksData(QVector<QString>& startNodeIDs, QVector<QString> endNodeIDs);

    /**
     * Slot for updating the trains plot.
     *
     * @param trainsStartEndPoints  The start and end points of trains.
     *
     * @author	Ahmed Aredah
     * @date	6/7/2023
     */
    void updateTrainsPlot(Vector<std::pair<std::string, Vector<std::pair<double,double>>>> trainsStartEndPoints);

    /**
     * Slot for closing the application.
     *
     * @author	Ahmed Aredah
     * @date	6/7/2023
     */
    void closeApplication();

    /**
     * Slot for clearing the form.
     *
     * @author	Ahmed Aredah
     * @date	6/7/2023
     */
    void clearForm();

    /**
     * @brief Slot for handling loading the sample project files
     *
     * @author	Ahmed Aredah
     * @date	6/7/2023
     */
    void handleSampleProject();

    void pauseSimulation();
    void resumeSimulation();

private slots:
    /**
     * Slot for showing the report.
     *
     * @author	Ahmed Aredah
     * @date	6/7/2023
     */
    void showReport();

    /**
     * Slot for setting a value in the report.
     *
     * @param recNo         The record number.
     * @param paramName     The parameter name.
     * @param paramValue    The parameter value.
     * @param reportPage    The report page number.
     *
     * @author	Ahmed Aredah
     * @date	6/7/2023
     */
    void setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage);

    // /**
    //  * Slot for setting the data set information.
    //  *
    //  * @param dsInfo    The data set information.
    //  *
    //  * @author	Ahmed Aredah
    //  * @date	6/7/2023
    //  */
    // void setDSInfo(DataSetInfo &dsInfo);

    /**
     * Slot for handling errors that occur during the simulation.
     *
     * @param error     The error message.
     *
     * @author	Ahmed Aredah
     * @date	6/7/2023
     */
    void handleError(std::string error);

    /**
     * Slot for handling the completion of the simulation.
     *
     * @param summaryData       The summary data of the simulation.
     * @param trajectoryFile    The trajectory file generated by the simulation.
     *
     * @author	Ahmed Aredah
     * @date	6/7/2023
     */
    void handleSimulationFinished(TrainsResults results);

    /**
     * Slot for updating the 'visualize trains' combo box.
     *
     * @author	Ahmed Aredah
     * @date	6/7/2023
     */
    void updateCombo_visualizeTrains();

    /**
     * Slot for handling the selection of a train point on the plot.
     *
     * @param selectedPoint     The selected point coordinates.
     *
     * @author	Ahmed Aredah
     * @date	6/7/2023
     */
    void trainPointSelected(QPointF selectedPoint);

    /**
     * Slot for handling the deletion of a train point on the plot.
     *
     * @param selectedPoint     The selected point coordinates.
     *
     * @author	Ahmed Aredah
     * @date	6/7/2023
     */
    void trainPointDeleted(QPointF selectedPoint);

    void updateTheNodesPlotData();

    void updateTheLinksPlotData();

    void addRowToNewNode();

    void addRowToNewLinks();

public:

    /**
     * @brief default Browse Path
     */
    QString defaultBrowsePath;

    QString userBrowsePath;

    /**
     * @brief load default settings
     *
     * @author	Ahmed Aredah
     * @date	7/1/2023
     */
    void loadDefaults();

    /**
     * @brief NeTrainSim::save Default values
     * @param defaults
     * @return
     */
    bool saveDefaults(QStringList defaults);

    /**
     * Constructor
     *
     * @param [in,out]	parent	(Optional) If non-null, the parent.
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    NeTrainSim(QWidget *parent = nullptr);

    /**
     * Destructor.
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    ~NeTrainSim();

    /**
    * Displays a notification with the given text for 3000 millisecond.
    * @param text The text of the notification to be displayed.
    *
    * @author Ahmed Aredah
    * @date 6/7/2023
    */
    void showNotification(QString text);

    /**
    * Displays a warning message with the given text.
    * @param text The text of the warning message to be displayed.
    *
    * @author Ahmed Aredah
    * @date 6/7/2023
    */
    void showWarning(QString text);

    /**
     * @brief load the Project Files to the simulator GUI
     * @param projectFilename the file NTS that has the files address
     *
     * @author Ahmed Aredah
     * @date 6/7/2023
     */
    void loadProjectFiles(QString projectFilename);

    void replotHistoryNodes();

    void replotHistoryLinks();

    void forceReplotNodes();

    void forceReplotLinks();

    void loadTrainsDataToTables(
        Vector<Map<std::string, std::any>> trainsRecords);

private:
    // The user interface
    Ui::NeTrainSim *ui;

    // // Pointer to the SimulationWorker object used for running the simulation in a separate thread.
    // SimulationWorker* worker = nullptr;

    // // Pointer to the QThread object that is used for running the simulation worker.
    // QThread* thread = nullptr;

    // Vector to hold the labels displayed on the plot.
    // Each label is represented by a QCPItemText object.
    QVector<QCPItemText*> labelsVector;

    // Map to hold the coordinates of the network nodes.
    // The keys are node IDs (QString), and the values are pairs of x
    // and y coordinates (std::pair<double, double>).
    Map<QString, std::pair<double, double>> networkNodes;

    // QVector to store the x-coordinates of the nodes.
    QVector<double> nodesXData;

    // QVector to store the y-coordinates of the nodes.
    QVector<double> nodesYData;

    // QVector to store the labels of the nodes.
    QVector<QString> nodesLabelData;

    // QVector to store the start node IDs of the links.
    QVector<QString> linksStartNodeIDs;

    // QVector to store the end node IDs of the links.
    QVector<QString> linksEndNodeIDs;

    // Pointer to the AboutWindow object used to display information about the application.
    std::shared_ptr<AboutWindow> aboutWindow = nullptr;

    std::shared_ptr<settingsWindow> theSettingsWindow = nullptr;

    // String to store the project name.
    QString projectName;

    // String to store the author's name.
    QString author;

    // String to store the network name.
    QString networkName;

    // String to store the filename of the nodes data.
    QString nodesFilename;

    // String to store the filename of the links data.
    QString linksFilename;

    // String to store the filename of the trains data.
    QString trainsFilename;

    // String to store the filename of the project file.
    QString projectFileName;

    // Vector of pairs to store the summary data of the trains after simulation.
    // Each pair represents the train ID (std::string) and its corresponding summary data (std::string).
    QVector<QPair<QString, QString>> trainsSummaryData;

    // // holds the summary report
    // QtRPT * report = nullptr;

    // // holds the summary report printer
    // QPrinter *printer = nullptr;

    ConfigurationManager* configManager;

    void showDetailedReport(QString trajectoryFilename);

    void drawLineGraph(CustomPlot &plot, const QVector<double> &xData,
                       const QVector<double> &yData, QString xLabel,
                       QString yLabel, QString graphName, int plotIndex);



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

    /**
     * Sets up the nodes table in the UI.
     * This function initializes and populates the nodes table with the appropriate columns and data.
     */
    void setupNodesTable();

    /**
    * Sets up the links table in the UI.
    * This function initializes and populates the links table with the appropriate columns and data.
    */
    void setupLinksTable();

    /**
     * Sets up the locomotives table in the UI.
     * This function initializes and populates the locomotives table with the appropriate columns and data.
     */
    void setupLocomotivesTable();

    /**
     * Sets up the cars table in the UI.
     * This function initializes and populates the cars table with the appropriate columns and data.
     */
    void setupCarsTable();

    /**
     * Sets up the configurations table in the UI.
     * This function initializes and populates the configurations table with the appropriate columns and data.
     */
    void setupConfigurationsTable();

    /**
    * Sets up the trains table in the UI.
    * This function initializes and populates the trains table with the appropriate columns and data.
    */
    void setupTrainsTable();

    /**
    * Retrieves nodes data from the nodes table in the UI.
    * This function extracts the nodes' data from the nodes table and returns it as a vector of tuples.
    *
    * @return A vector of tuples representing the nodes' data. Each tuple contains the node ID, x-coordinate, y-coordinate,
    *         node type, length, and width.
    */
    Vector<Map<string, string> > getNodesDataFromNodesTable();

    /**
    * Converts nodes data to plottable format.
    * This function takes a vector of node records and converts it to the format suitable for plotting.
    * It returns a tuple containing the x-coordinates, y-coordinates, and labels of the nodes.
    *
    * @param nodeRecords The vector of node records to convert.
    * @return A tuple of QVector objects representing the x-coordinates, y-coordinates, and labels of the nodes.
    */
    std::tuple<QVector<double>, QVector<double>, QVector<QString>>
                    getNodesPlottableData(Vector<Map<std::string,
                                     std::string>>& nodeRecords);

    /**
    * Updates the nodes plot with new data.
    * This function updates the specified QCustomPlot object with the given x-coordinates, y-coordinates, and labels,
    * and redraws the plot.
    *
    * @param plot The QCustomPlot object to update.
    * @param xData The x-coordinates of the nodes.
    * @param yData The y-coordinates of the nodes.
    * @param labels The labels of the nodes.
    * @param showLabels Flag indicating whether to show labels on the plot. Default is false.
    */
    void updateNodesPlot(CustomPlot& plot, QVector<double> xData,
                         QVector<double> yData, QVector<QString> labels,
                         bool showLabels = false);

    /**
    * Retrieves links data from a links file.
    * This function reads the links data from the specified file and returns it as a vector of tuples.
    *
    * @param fileName The name of the links file.
    * @return A vector of tuples representing the links data. Each tuple contains the start node ID, end node ID, link type,
    *         length, width, lane count, speed limit, and capacity.
    */
    Vector<Map<std::string, std::string>>
    getLinkesDataFromLinksFile(QString fileName);

    /**
    * Retrieves links data from the links table in the UI.
    * This function extracts the links' data from the links table and returns it as a vector of tuples.
    *
    * @return A vector of tuples representing the links' data. Each tuple contains the start node ID, end node ID, link type,
    *         length, width, lane count, speed limit, and capacity.
    */
    Vector<Map<std::string, std::string>> getLinkesDataFromLinksTable();

    /**
    * Converts links data to plottable format.
    * This function takes a vector of link records and converts it to the format suitable for plotting.
    * It returns a tuple containing the start node IDs and end node IDs of the links.
    *
    * @param linksRecords The vector of link records to convert.
    * @return A tuple of QVector objects representing the start node IDs and end node IDs of the links.
    */
    std::tuple<QVector<QString>, QVector<QString>>
                    getLinksPlottableData(Vector<Map<std::string,
                                     std::string>> linksRecords);

    /**
    * Updates the links plot with new data.
    * This function updates the specified QCustomPlot object with the given start node IDs and end node IDs of the links,
    * and redraws the plot.
    *
    * @param plot The QCustomPlot object to update.
    * @param startNodeIDs The start node IDs of the links.
    * @param endNodeIDs The end node IDs of the links.
    */
    void updateLinksPlot(CustomPlot& plot, QVector<QString> startNodeIDs, QVector<QString> endNodeIDs);

    /**
    * Opens a file browser dialog and returns the selected file name.
    *
    * @param theLineEdit The QLineEdit widget to update with the selected file name.
    * @param theFileName The default file name or filter for the file browser dialog.
    * @return The selected file name.
    */
    QString browseFiles(QLineEdit* theLineEdit, const QString& theFileName);

    /**
    * Retrieves nodes data from a nodes file.
    * This function reads the nodes data from the specified file and returns it as a vector of tuples.
    *
    * @param fileName The name of the nodes file.
    * @return A vector of tuples representing the nodes data. Each tuple contains the node ID, x-coordinate, y-coordinate,
    *         node type, length, and width.
    */
    Vector<Map<std::string, std::string>>
    getNodesDataFromNodesFile(QString fileName);

    /**
    * Opens a folder browser dialog and updates the specified QLineEdit widget with the selected folder path.
    *
    * @param theLineEdit The QLineEdit widget to update with the selected folder path.
    * @param theHelpMessage The help message to display in the folder browser dialog.
    */
    void browseFolder(QLineEdit* theLineEdit, const QString& theHelpMessage);

    /**
    * Retrieves trains data from the tables in the UI.
    * This function reads the trains data from the tables and returns it as a vector of tuples.
    *
    * @return A vector of tuples representing the trains data. Each tuple contains the train ID, route nodes, start time,
    *         end time, speed profile, acceleration profile, and whether the train is enabled.
    */
    Vector<Map<string, std::any>> getTrainsDataFromTables();

    /**
    * Initiates the simulation process.
    * This function starts the simulation based on the selected trains
    * data and updates the UI accordingly.
    */
    void simulate();

    /**
    * Finds a label by its position in the specified QCustomPlot object.
    *
    * @param plot The QCustomPlot object to search for the label in.
    * @param targetPosition The target position of the label.
    * @return A pointer to the QCPItemText object representing the
    * label if found, nullptr otherwise.
    */
    QCPItemText* findLabelByPosition(CustomPlot* plot,
                                     const QPointF& targetPosition);



    void loadNodesDataToTable(
        Vector<Map<std::string, std::string>> nodesRecords);

    void loadLinksDataToTable(
        Vector<Map<std::string, std::string>> linksRecords);

    void loadLocomotivesDataToTable(
        Vector<Map<std::string, std::any>> trainsRecords);

    void loadCarsDataToTable(
        Vector<Map<std::string, std::any>> trainsRecords);

    void loadTrainConfigsToTable(
        Vector<Map<std::string, std::any>> trainsRecords);

    void loadTrainsDataToTable(
        Vector<Map<std::string, std::any>> trainsRecords);

    void loadConfigsDataToTable(
        Vector<Map<std::string, std::any>> trainsRecords);

    int getCarIDFromTable(
        Map<std::string, std::string> carRecords);

    int getLocomotiveIDFromTable(
        Map<std::string, std::string> locomotiveRecords);


protected:
    /**

    * Event handler for the close event of the NeTrainSim window.
    * @param event Pointer to the QCloseEvent object.
    * @author Ahmed Aredah
    * @date 6/7/2023
    */
    void closeEvent(QCloseEvent* event) override;
};
#endif // NETRAINSIMMAINWINDOW_H
