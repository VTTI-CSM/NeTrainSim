#pragma once

#include <QObject>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QPointF>

/**
  * \abstract CornerGrabber is a graphics item which allows the user to grab a corner of a square,
  *           or ends of a line. The grabber captures the mouse when hovered, and makes itself
  *           visible when hovered, and makes itself RED when clicked.
  *
  */

class CornerGrabber : public QGraphicsItem
{
public:
    explicit CornerGrabber(QGraphicsItem *parent = 0,  int corner = 0);

    int  getCorner(); // allows the owner to find out which coner this is
    void setMouseState(int); // allows the owner to record the current mouse state
    int  getMouseState(); // allows the owner to get the current mouse state
    QPointF getCenterPoint();

    qreal mouseDownX;
    qreal mouseDownY;

    enum {kMouseReleased=0, kMouseDown, kMouseMoving}; //  define the mouse states

    virtual QRectF boundingRect() const Q_DECL_OVERRIDE; // must be re-implemented in this class to provide the diminsions of the box to the QGraphicsView
    void setCursorToResize(bool on);

private:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE; // must be re-implemented to handle mouse hover enter events
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE; // must be re-implemented to handle mouse hover leave events

    // once the hover event handlers are implemented in this class,
    // the mouse events must allow be implemented because of
    // some linkage issue - apparrently there is some connection
    // between the hover events and mouseMove/Press/Release
    // events which triggers a vtable issue
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;

    void mouseMoveEvent(QGraphicsSceneDragDropEvent *event);
    void mousePressEvent(QGraphicsSceneDragDropEvent *event);

    QColor m_outterborderColor;   // the hover event handlers will toggle this between red and black
    QPen m_outterborderPen;       // the pen is used to paint the red/black border

    qreal   m_width;
    qreal   m_height;

    int m_corner;  // 0,1,2,3  - starting at x=0,y=0 and moving clockwise around the box
    int m_mouseButtonState;

};

using SPtrCorner = QSharedPointer<CornerGrabber>;
