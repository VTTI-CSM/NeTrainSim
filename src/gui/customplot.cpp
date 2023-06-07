#include "src/gui/customplot.h"
#include "src/util/Vector.h"

CustomPlot::CustomPlot(QWidget *parent) : QCustomPlot(parent), m_isPanning(false), m_isScrollButtonClicked(false)
{
    // Connect the zoomReset signal to the resetZoom slot
    QObject::connect(this, SIGNAL(zoomReset()), this, SLOT(resetZoom()));
}

void CustomPlot::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
    {
        // get the position of the mouse for the panning
        if (axisRect()->rect().contains(event->pos()))
        {
            m_curXRange = xAxis->range().size();
            m_curYRange = yAxis->range().size();
            m_x0 = event->pos().x();
            m_y0 = event->pos().y();
            m_xPress = xAxis->pixelToCoord(m_x0);
            m_yPress = yAxis->pixelToCoord(m_y0);
            m_isPanning = true;
            panningSensitivity = this->calculateSensitivity();
        }
    }

    QCustomPlot::mousePressEvent(event);
}

void CustomPlot::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button()== Qt::MiddleButton) {
        emit zoomReset();
    }

     QCustomPlot::mouseDoubleClickEvent(event);
}

void CustomPlot::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPanning)
    {
        double dx = event->pos().x() - m_x0;
        double dy = event->pos().y() - m_y0;
        double xPressNew = xAxis->pixelToCoord(m_x0 + dx);
        double yPressNew = yAxis->pixelToCoord(m_y0 + dy);
        double dxPress = xPressNew - m_xPress;
        double dyPress = yPressNew - m_yPress;

        double xRangeChange = -(dxPress * m_curXRange / axisRect()->width()) * this->panningSensitivity;
        double yRangeChange = -(dyPress * m_curYRange / axisRect()->height()) * this->panningSensitivity;

        xAxis->setRange(xAxis->range().lower + xRangeChange, xAxis->range().upper + xRangeChange);
        yAxis->setRange(yAxis->range().lower + yRangeChange, yAxis->range().upper + yRangeChange);

        replot();
    }

    QCustomPlot::mouseMoveEvent(event);
}


void CustomPlot::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton && m_isPanning)
    {
        m_isPanning = false;
    }

    QCustomPlot::mouseReleaseEvent(event);
}

void CustomPlot::wheelEvent(QWheelEvent *event)
{
    if (event->buttons() == Qt::MiddleButton)
    {
        // Center the drawing when the scroll button is clicked
        if (event->angleDelta().y() == 0)
        {
            centerDrawing();
            return;
        }
    }

    // Calculate the scaling factor
    double scaleFactor = 1.15;
    double factor = event->angleDelta().y() > 0 ? (1.0 / scaleFactor) : scaleFactor;

    // Calculate the new axis ranges based on the mouse position
    double x = xAxis->pixelToCoord(event->position().x());
    double y = yAxis->pixelToCoord(event->position().y());
    double xRange = xAxis->range().size() * factor;
    double yRange = yAxis->range().size() * factor;
    xAxis->setRange(x - (x - xAxis->range().lower) * (xRange / xAxis->range().size()), x + (xAxis->range().upper - x) * (xRange / xAxis->range().size()));
    yAxis->setRange(y - (y - yAxis->range().lower) * (yRange / yAxis->range().size()), y + (yAxis->range().upper - y) * (yRange / yAxis->range().size()));

    replot();
    QCustomPlot::wheelEvent(event);
}

void CustomPlot::centerDrawing()
{
    // Calculate the center coordinates of the graph
    double centerX = (xAxis->range().lower + xAxis->range().upper) / 2.0;
    double centerY = (yAxis->range().lower + yAxis->range().upper) / 2.0;

    // Set new axis ranges to center the drawing
    xAxis->setRange(centerX - (xAxis->range().size() / 2.0), centerX + (xAxis->range().size() / 2.0));
    yAxis->setRange(centerY - (yAxis->range().size() / 2.0), centerY + (yAxis->range().size() / 2.0));

    replot();
}

void CustomPlot::resetZoom()
{
    if (graphCount() < 1) {
        return;
    }

    Vector<double> minXV, maxXV, minYV, maxYV;

    for (int i = 0; i < graphCount(); i++) {
        auto data = this->getAllPointsPositions(*graph(i));
        auto xdata = data.first;
        auto ydata = data.second;

        if (xdata.size() < 1 || ydata.size() < 1) {
            continue;  // Skip empty data sets
        }
        // Get the maximum and minimum values of the plotted points
        minXV.push_back(xdata.min());
        maxXV.push_back(xdata.max());
        minYV.push_back(ydata.min());
        maxYV.push_back(ydata.max());
    }

    if (minXV.empty() || maxXV.empty() || minYV.empty() || maxYV.empty()) {
        return;  // Skip if all vectors are empty
    }

    double minX = minXV.min();
    double maxX = maxXV.max();
    double minY = minYV.min();
    double maxY = maxYV.max();

    // Calculate the percentage of the difference
    double xRange = (maxX - minX) * 0.1;  // Adjust the percentage as desired
    double yRange = (maxY - minY) * 0.1;  // Adjust the percentage as desired

    // Set the range based on the maximum and minimum values
    xAxis->setRange(minX - xRange, maxX + xRange);
    yAxis->setRange(minY - yRange, maxY + yRange);

    replot();
}


std::pair<Vector<double>, Vector<double>> CustomPlot::getAllPointsPositions(QCPGraph &graph)
{
    Vector<double> xdata;
    Vector<double> ydata;

    // Get the data of the graph
    QSharedPointer< QCPGraphDataContainer >  data = graph.data();

    // Iterate over the data points of the graph
    for (auto &graphData : *data)
    {
        double x = graphData.key;
        double y = graphData.value;
        xdata.push_back(x);
        ydata.push_back(y);
    }
    return std::make_pair(xdata, ydata);
}

double CustomPlot::calculateSensitivity()
{
    const double sensitivityMin = 0.01;
    const double sensitivityMax = 0.5;

    double xRange = xAxis->range().size();
    double yRange = yAxis->range().size();

    // Calculate the minimum and maximum values of the x-axis range
    double xRangeMin = xAxis->range().lower;
    double xRangeMax = xAxis->range().upper;

    // Calculate the average range
    double averageRange = (xRange + yRange) / 2.0;

    // Map the average range to the sensitivity range
    double mappedSensitivity = sensitivityMin + (sensitivityMax - sensitivityMin) * (averageRange - xRangeMin) / (xRangeMax - xRangeMin);

    return std::abs(mappedSensitivity);
}