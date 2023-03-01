#include "GraphicsLine.h"
#include <QBrush>
#include <QLinearGradient>
#include <QDebug>
#include <QRect>
#include <QPolygon>
#include <QGraphicsPathItem>
#include <QApplication>
#include "GraphicsScene.h"
#include "math.h"

GraphicsLine::GraphicsLine()
: QGraphicsPolygonItem(),
m_cornerGrabbed(false)
{
    setDrawingState(true);

    m_corners.resize(2);
    for (auto &corner : m_corners)
        corner = (SPtrCorner)nullptr;

    setFlag(QGraphicsItem::ItemIsSelectable,true);
    setFlag(ItemSendsGeometryChanges,true);

    m_outterborderPen.setWidth(2);
    m_outterborderPen.setColor(m_outterborderColor);

    this->setAcceptHoverEvents(true);

    this->graphicsItem = this;
}

void GraphicsLine::initPolygon()
{
    quint16 polygonWidth = 4;
    QPolygonF selectRegion;
    for (int p = 0; p < m_pointList.size(); p++) {
        QPointF p1(m_pointList[p].x() - polygonWidth, m_pointList[p].y() - polygonWidth);
        QPointF p2(m_pointList[p].x() + polygonWidth, m_pointList[p].y() - polygonWidth);

        QPointF p3(m_pointList[p].x() + polygonWidth, m_pointList[p].y() + polygonWidth);
        QPointF p4(m_pointList[p].x() - polygonWidth, m_pointList[p].y() + polygonWidth);

        selectRegion << p1 << p2 << p3 << p4 << p1 << p2;
    }

    for (int p = m_pointList.size()-1; p > -1; p--) {
        QPointF p3(m_pointList[p].x() + polygonWidth, m_pointList[p].y() + polygonWidth);
        QPointF p4(m_pointList[p].x() - polygonWidth, m_pointList[p].y() + polygonWidth);
        selectRegion << p3 << p4;
    }

    QPointF p1(m_pointList[0].x() - polygonWidth, m_pointList[0].y() - polygonWidth);
    selectRegion << p1;

    this->setPolygon(selectRegion);
    this->update();
}

/**
  * This scene event filter has been registered with all four corner grabber items.
  * When called, a pointer to the sending item is provided along with a generic
  * event.  A dynamic_cast is used to determine if the event type is one of the events
  * we are interrested in.
  */
bool GraphicsLine::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    //qDebug() << " QEvent == " + QString::number(event->type());

    CornerGrabber *corner = dynamic_cast<CornerGrabber *>(watched);
    if (corner == nullptr) return false; // not expected to get here

    QGraphicsSceneMouseEvent * mevent = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
    if (mevent == nullptr) {
        // this is not one of the mouse events we are interrested in
        return false;
    }

    switch (event->type()) {
        // if the mouse went down, record the x,y coords of the press, record it inside the corner object
    case QEvent::GraphicsSceneMousePress: {
        corner->setMouseState(CornerGrabber::kMouseDown);

        QPointF scenePosition =  corner->mapToScene(mevent->pos());
        corner->mouseDownY = scenePosition.y();
        corner->mouseDownX= scenePosition.x();

        m_cornerGrabbed = true;

        //Посылаем сигнал в сцену для отслеживания Ундо при перемещении концов
        auto model = qobject_cast<GraphicsScene *>(scene());
        if (model)
        	model->itemMoving(this);
        break;
	}
    case QEvent::GraphicsSceneMouseRelease: {
        corner->setMouseState(CornerGrabber::kMouseReleased);
        m_cornerGrabbed = false;
        break;
	}
    case QEvent::GraphicsSceneMouseMove: {
        corner->setMouseState(CornerGrabber::kMouseMoving );
        break;
	}
    default:
        // we dont care about the rest of the events
        return false;
        break;
    }

    if ( corner->getMouseState() == CornerGrabber::kMouseMoving ) {
        auto m_scene = qobject_cast<GraphicsScene *>(scene());
        m_scene->itemResizing(this);

        createCustomPath(mevent->pos(), corner);
        this->scene()->update();
    }

    return true; // true => do not send event to watched - we are finished with this event
}

void GraphicsLine::setPos(QPointF pos)
{
    m_location = pos;
    QGraphicsItem::setPos(pos);
}

void GraphicsLine::setPos(qreal x, qreal y)
{
    setPos(QPoint(x,y));
}

void GraphicsLine::setEndPosition(QPointF endPoint)
{
    m_pointList[m_pointList.size()-1] = endPoint;
    initPolygon();
    setCornerPositions();
}

void GraphicsLine::createCustomPath(QPointF mouseLocation, CornerGrabber* corner)
{
    if (corner == nullptr)
        return;
    QPointF scenePosition = corner->mapToScene(mouseLocation);

    // which corner needs to get moved?
    int idx = -1;
    for (int p = 0; p < m_pointList.size(); p++) {
        if (corner == m_corners[p]) {
            idx = p;
            m_pointList[p] = mapFromScene(scenePosition);
        }
    }
    initPolygon();
    if (idx > -1) {
        int cornerWidth = (m_corners[0]->boundingRect().width())/2;
        int cornerHeight = (m_corners[0]->boundingRect().height())/2;
        m_corners[idx]->setPos(m_pointList[idx].x() - cornerWidth, m_pointList[idx].y() - cornerHeight );
    }
}

void GraphicsLine::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    this->scene()->update();
}

void GraphicsLine::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    this->scene()->update();
}

// for supporting moving the box across the scene
void GraphicsLine::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    event->setAccepted(true);

    auto model = qobject_cast<GraphicsScene *>(scene());
    int numMoved = model->itemsMoved().size();

    if (numMoved > 1) {
        qDebug() << model->selectedItems().size();
        qDebug() << model->itemsSelected().size();
        return;
    }

    this->setSelected(this->isSelected());
}

// for supporting moving the box across the scene
void GraphicsLine::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    event->setAccepted(true);

    if (event->button() == Qt::LeftButton)
        m_dragStart = event->pos();

    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);

    if (QApplication::keyboardModifiers() == Qt::ControlModifier)
        this->setSelected(!this->isSelected());
    else {
        if (this->scene()->selectedItems().size() == 1)
            this->setSelected(true);
    }
}

// for supporting moving the box across the scene
void GraphicsLine::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (this->type() == ItemType::GBand)
        return;

    if (event->buttons() & Qt::LeftButton) {
        int distance = (event->pos() - m_dragStart).manhattanLength();
        if (distance < QApplication::startDragDistance())
            return;
    }


    QGraphicsItem::mouseMoveEvent(event); // move the item...

    auto m_scene = qobject_cast<GraphicsScene *>(scene());

    auto band = qgraphicsitem_cast<ReportBand*>(this->parentItem());
    if (band != nullptr) {
        m_scene->itemResizing(this);
    }

    QPointF newPos = event->pos() ;
    m_location += (newPos - m_dragStart);
    this->setPos(m_location);
    this->scene()->update();
}

void GraphicsLine::setSelected(bool selected_)
{
    QGraphicsItem::setSelected(selected_);
    if (itemInTree != nullptr)
        itemInTree->setSelected(selected_);

    if (selected_) {
        createCorners();
        auto m_scene = qobject_cast<GraphicsScene *>(scene());
        m_scene->itemSelect(this);
        this->setZValue(3);
    } else {
        destroyCorners();
        this->setZValue(2);
    }

    this->scene()->update();
}

bool GraphicsLine::isSelected()
{
    if (itemInTree != nullptr)
        return itemInTree->isSelected();

    return false;
}

// create the corner grabbers
void GraphicsLine::createCorners()
{
    m_outterborderColor = m_borderColor;

    for (int p = 0; p < m_pointList.size(); p++) {
        if (m_corners[p] == nullptr) {
            m_corners[p] = SPtrCorner(new CornerGrabber(this, p));
            m_corners[p]->installSceneEventFilter(this);
        }
    }

    setCornerPositions();
}

void GraphicsLine::setCornerPositions()
{
    if (m_corners[0] == nullptr || m_corners[1] == nullptr)
        return;
    int cornerWidth = (m_corners[0]->boundingRect().width())/2;
    int cornerHeight = (m_corners[0]->boundingRect().height())/2;

    for (int p = 0; p < m_pointList.size(); p++) {
        if (m_corners[p] != nullptr)
            m_corners[p]->setPos(m_pointList[p].x() - cornerWidth, m_pointList[p].y() - cornerHeight );
    }
}

#if 0
QRectF Transistion::boundingRect() const {
    return QRectF(0,0,_graphicsItemBoundingBoxWidth,_height);
}
#endif

void GraphicsLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *i, QWidget *w)
{
    Q_UNUSED(i);
    Q_UNUSED(w);

    if (getDrawingState()) {
        m_outterborderPen.setStyle(borderStyle());
        m_outterborderPen.setColor(m_borderColor);
        m_outterborderPen.setWidth(getBorderWidth());
        painter->setPen(m_outterborderPen);

        QBrush brush2(m_borderColor,Qt::SolidPattern);

        QPainterPath path;
        path.moveTo(m_pointList.at(0));

        for (auto &point : m_pointList)
            path.lineTo(point);

        painter->drawPath(path);

        /*
        QPen _pen = this->pen();
        _pen.setStyle(Qt::DotLine);
        _pen.setColor(Qt::red);
        painter->setPen(_pen);
        painter->drawPolygon(this->polygon());*/

        // Draw the arrows
        static const double Pi = 3.14159265358979323846264338327950288419717;
        static double TwoPi = 2.0 * Pi;
        QLineF line(m_pointList[0], m_pointList[1]);
        double angle = ::acos(line.dx() / line.length());
        if (line.dy() >= 0)
             angle = TwoPi - angle;

        int arrowSize= 10;

        if (m_arrowStart) {
            QPointF sourceArrowP1 = m_pointList[0] + QPointF(sin(angle + Pi / 3) * arrowSize,
                                                          cos(angle + Pi / 3) * arrowSize);
            QPointF sourceArrowP2 = m_pointList[0] + QPointF(sin(angle + Pi - Pi / 3) * arrowSize,
                                                          cos(angle + Pi - Pi / 3) * arrowSize);
            QPointF mPoint = line.pointAt(5/line.length());

            QPolygonF polygon;
            polygon << m_pointList[0] << sourceArrowP1 << mPoint << sourceArrowP2;
            painter->drawPolygon(polygon);

            QPainterPath path;
            path.addPolygon(polygon);
            painter->fillPath(path,brush2);
        }
        if (m_arrowEnd) {
            QPointF destArrowP1 = m_pointList[1] + QPointF(sin(angle - Pi / 3) * arrowSize,
                                                      cos(angle - Pi / 3) * arrowSize);
            QPointF destArrowP2 = m_pointList[1] + QPointF(sin(angle - Pi + Pi / 3) * arrowSize,
                                                      cos(angle - Pi + Pi / 3) * arrowSize);

            QPointF mPoint = line.pointAt(1-5/line.length());

            QPolygonF polygon;
            polygon << m_pointList[1] << destArrowP1 << mPoint << destArrowP2;
            painter->drawPolygon(polygon);

            QPainterPath path;
            path.addPolygon(polygon);
            painter->fillPath(path,brush2);
        }
    }
}

void GraphicsLine::mouseMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    event->setAccepted(false);
}

void GraphicsLine::mousePressEvent(QGraphicsSceneDragDropEvent *event)
{
    event->setAccepted(false);
}

QVariant GraphicsLine::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange) {
        auto model = qobject_cast<GraphicsScene *>(scene());
        if (model)
            model->itemMoving(this);
    }

    return QGraphicsItem::itemChange(change, value);
}

GraphicsLine *GraphicsLine::clone()
{
    QPoint newPos(this->x(),this->y());
    newPos.setY(newPos.y()+5);
    newPos.setX(newPos.x()+5);

    auto newLine  = new GraphicsLine();
    newLine->setBorderColor(m_borderColor);
    newLine->setFieldType(this->getFieldType());
    newLine->setPos(newPos);
    newLine->setPointList(getPointList());
    newLine->setArrow(ArrowStart, this->getArrow(ArrowStart));
    newLine->setArrow(ArrowEnd, this->getArrow(ArrowEnd));
    newLine->setColorValue(BorderColor,this->getColorValue(BorderColor));
    newLine->setBorderWidth(this->getBorderWidth());
    return newLine;
}

void GraphicsLine::loadParamFromXML(QDomElement e)
{
    GraphicsHelperClass::loadParamFromXML(e);

	m_pointList.clear();
    m_pointList << QPointF(e.attribute("lineStartX").toInt(), e.attribute("lineStartY").toInt()+20);
    m_pointList << QPointF(e.attribute("lineEndX").toInt(), e.attribute("lineEndY").toInt()+20);
    this->m_arrowStart = e.attribute("arrowStart","0").toInt();
    this->m_arrowEnd = e.attribute("arrowEnd","0").toInt();

    this->setZValue(2);
	initPolygon();
}

QDomElement GraphicsLine::saveParamToXML(QSharedPointer<QDomDocument> xmlDoc)
{
    QDomElement elem = GraphicsHelperClass::saveParamToXML(xmlDoc);

    elem.setAttribute("lineStartX", (int)mapToParent(m_pointList[0]).x());
    elem.setAttribute("lineStartY", (int)mapToParent(m_pointList[0]).y()-20);
    elem.setAttribute("lineEndX", (int)mapToParent(m_pointList[1]).x());
    elem.setAttribute("lineEndY", (int)mapToParent(m_pointList[1]).y()-20);

    elem.setAttribute("arrowStart",this->m_arrowStart);
    elem.setAttribute("arrowEnd",this->m_arrowEnd);

    return elem;
}

void GraphicsLine::setArrow(QtRptName::Command command, QVariant value)
{
    if (command == QtRptName::ArrowStart)
        m_arrowStart = value.toBool();
    if (command == QtRptName::ArrowEnd)
        m_arrowEnd = value.toBool();
    this->update();
}

bool GraphicsLine::getArrow(QtRptName::Command command)
{
    bool result = false;
    if (command == QtRptName::ArrowStart)
        result = m_arrowStart;
    if (command == QtRptName::ArrowEnd)
        result = m_arrowEnd;
    return result;
}

qreal GraphicsLine::getLength()
{
    QLineF line(m_pointList[0], m_pointList[1]);
    return line.length();
}

void GraphicsLine::setLength(qreal value)
{
    QLineF line(m_pointList[0], m_pointList[1]);
    line.setLength(value);
    m_pointList[0] = line.p1();
    setEndPosition(line.p2());
}

void GraphicsLine::setMenu(QMenu *menu)
{
    QIcon icon;

    auto actContDel = new QAction(tr("Delete"),this);
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/delete.png")), QIcon::Normal, QIcon::On);
    actContDel->setObjectName("actContDel");
    actContDel->setIcon(icon);
    QObject::connect(actContDel, SIGNAL(triggered()), this, SIGNAL(itemRemoving()));

    auto actContMoveForward = new QAction(tr("Move forward"),this);
    actContMoveForward->setObjectName("actContMoveForward");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/moveForward.png")), QIcon::Normal, QIcon::On);
    actContMoveForward->setIcon(icon);
    QObject::connect(actContMoveForward, SIGNAL(triggered()), this, SLOT(moveForward()));

    auto actContMoveBack = new QAction(tr("Move back"),this);
    actContMoveBack->setObjectName("actContMoveBack");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/moveBack.png")), QIcon::Normal, QIcon::On);
    actContMoveBack->setIcon(icon);
    QObject::connect(actContMoveBack, SIGNAL(triggered()), this, SLOT(moveBack()));

    m_menu->clear();
    m_menu->insertActions(0, menu->actions());
    m_menu->addAction(actContDel);
    m_menu->addSeparator();
    m_menu->addAction(actContMoveForward);
    m_menu->addAction(actContMoveBack);
}
