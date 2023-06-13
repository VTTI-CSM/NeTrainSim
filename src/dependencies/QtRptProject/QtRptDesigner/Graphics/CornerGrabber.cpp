#include "CornerGrabber.h"
#include <QApplication>
#include <QDebug>

CornerGrabber::CornerGrabber(QGraphicsItem *parent,  int corner)
: QGraphicsItem(parent),
    mouseDownX(0),
    mouseDownY(0),
    m_outterborderColor(Qt::black),
    m_outterborderPen(),
    m_width(6),
    m_height(6),
    m_corner(corner),
    m_mouseButtonState(kMouseReleased)
{
    setParentItem(parent);

    m_outterborderPen.setWidth(2);
    m_outterborderPen.setColor(m_outterborderColor);

   this->setAcceptHoverEvents(true);
}

void CornerGrabber::setMouseState(int s)
{
    m_mouseButtonState = s;
}

int CornerGrabber::getMouseState()
{
    return m_mouseButtonState;
}

int CornerGrabber::getCorner()
{
    return m_corner;
}

// we have to implement the mouse events to keep the linker happy,
// but just set accepted to false since are not actually handling them
void CornerGrabber::mouseMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    event->setAccepted(false);
}

void CornerGrabber::mousePressEvent(QGraphicsSceneDragDropEvent *event)
{
    event->setAccepted(false);
}

void CornerGrabber::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->setAccepted(true);
}

void CornerGrabber::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->setAccepted(false);
}

void CornerGrabber::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    event->setAccepted(false);
}

// change the color on hover events to indicate to the use the object has
// been captured by the mouse
void CornerGrabber::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    m_outterborderColor = Qt::black;

    // reset the cursor
    this->setCursorToResize(false);
    this->update(0,0,m_width,m_height);
}

void CornerGrabber::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    m_outterborderColor = Qt::red;

    // change the cursor to the diagonal sizer
    this->setCursorToResize(true);
    this->update(0,0,m_width,m_height);
}

QRectF CornerGrabber::boundingRect() const
{
    return QRectF(0,0,m_width,m_height);
}

QPointF CornerGrabber::getCenterPoint()
{
    return QPointF(pos().x() + m_width/2,pos().y() + m_height/2);
}

void CornerGrabber::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    // fill the box with solid color, use sharp corners

    m_outterborderPen.setCapStyle(Qt::SquareCap);
    m_outterborderPen.setStyle(Qt::SolidLine);
    painter->setPen(m_outterborderPen);

    QPointF topLeft (0, 0);
    QPointF bottomRight (m_width, m_height);

    QRectF rect (topLeft, bottomRight);

    QBrush brush (Qt::SolidPattern);
    brush.setColor (m_outterborderColor);
    painter->fillRect(rect,brush);
}

void CornerGrabber::setCursorToResize(bool on)
{
    if (on) {
        switch(m_corner) {
        case 0:
        case 2:
            QApplication::setOverrideCursor(Qt::SizeFDiagCursor);
            break;
        case 1:
        case 3:
            QApplication::setOverrideCursor(Qt::SizeBDiagCursor);
            break;
        case 4:  //bottom center
        case 5:  //top center
            QApplication::setOverrideCursor(Qt::SizeVerCursor);
            break;
        case 6:  //left center
        case 7:  //rigth center
            QApplication::setOverrideCursor(Qt::SizeHorCursor);
            break;
        }
    } else {
        QApplication::restoreOverrideCursor();
    }
}
