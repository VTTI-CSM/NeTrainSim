#ifndef CUSTOMPLOT_H
#define CUSTOMPLOT_H

#include "src/dependencies/qcustomplot/qcustomplot.h"
#include "src/util/Vector.h"


class CustomPlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit CustomPlot(QWidget *parent = nullptr);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

private:
    bool m_isPanning;
    double m_x0, m_y0;
    double m_xPress, m_yPress;
    double m_curXRange, m_curYRange;
    bool m_isScrollButtonClicked;

    qint64  zoomResetTimeStamp;
    int zoomResetClickCounter;

    double panningSensitivity = 1.0;

private:
    void centerDrawing();
    std::pair<Vector<double>, Vector<double>> getAllPointsPositions(QCPGraph &graph);
    double calculateSensitivity();

signals:
    void zoomReset();

public slots:
    void resetZoom(); // Slot function for handling zoom reset
};

#endif // CUSTOMPLOT_H
