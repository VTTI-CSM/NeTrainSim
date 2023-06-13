#ifndef GRAPHICSLINE_H
#define GRAPHICSLINE_H

#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QTransform>
#include <QDomDocument>
#include <QGraphicsPolygonItem>
#include <QPainterPath>
#include "GraphicsNameSpace.h"
#include "GraphicsHelperClass.h"

class GraphicsLine : public QGraphicsPolygonItem, public GraphicsHelperClass
{
public:
    GraphicsLine();
    void setGridSpace(int space);
    qreal getLength();
    void setLength(qreal value);
    void setEndPosition(QPointF endPoint);
    virtual void setPos(QPointF pos);
    virtual void setPos(qreal x, qreal y);
    void setSelected(bool selected);
    bool isSelected();
    void setBorderColor(QColor value) {m_borderColor = value;}
    int type() const Q_DECL_OVERRIDE { return ItemType::GLine; }
    QList<QPointF> getPointList() {return m_pointList;}
    void setPointList(QList<QPointF> list) {m_pointList = list;}
    void addPoint(QPointF point) {m_pointList << point;}
    bool getArrowStart() {return m_arrowStart;}
    void setArrowStart(bool value) {m_arrowStart = value;}
    bool getArrowEnd() {return m_arrowEnd;}
    void setArrowEnd(bool value) {m_arrowEnd = value;}
    void setArrow(QtRptName::Command command, QVariant value);
    bool getArrow(QtRptName::Command command);
    GraphicsLine *clone();
    void loadParamFromXML(QDomElement e) Q_DECL_OVERRIDE;
    QDomElement saveParamToXML(QSharedPointer<QDomDocument> xmlDoc) Q_DECL_OVERRIDE;
    void setMenu(QMenu *menu);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE; // must be re-implemented here to pain the box on the paint-event
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE; // must be re-implemented to handle mouse hover enter events
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE; // must be re-implemented to handle mouse hover leave events

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;  // allows the main object to be moved in the scene by capturing the mouse move events
    void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;

    void mouseMoveEvent(QGraphicsSceneDragDropEvent *event);
    void mousePressEvent(QGraphicsSceneDragDropEvent *event);
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) Q_DECL_OVERRIDE;

    void createCorners();
    void setCornerPositions();
    void createCustomPath(QPointF mouseLocation, CornerGrabber*);

    QList<QPointF> m_pointList;

    bool m_cornerGrabbed;
    bool m_arrowStart;
    bool m_arrowEnd;
    void initPolygon();

};


#endif // GRAPHICSLINE_H
