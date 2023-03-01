// Copyright (C) 2008  Lukas Lalinsky
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include <QPainter>
#include "line.h"
#include "hub.h"
#include "diagramdocument.h"
#include "diagramobject.h"
#include "connector.h"

Line::Line(DiagramItem *parent)
: DiagramItem(parent), m_fillStartArrow(false), m_fillEndArrow(false), m_dirty(false)
{
	m_connectors[0] = new Connector(this);
	m_connectors[1] = new Connector(this);
	setZValue(1.0);
}

Line::~Line()
{
	delete m_connectors[0];
	delete m_connectors[1];
}

Connector *Line::connector(int index) const
{
	Q_ASSERT(index == 0 || index == 1);
	return m_connectors[index];
}

void Line::updateLayout()
{}


void Line::loadFromXml(QDomElement element, DiagramDocument *document)
{
    DiagramItem::loadFromXml(element, document);
    QDomElement connectorElement = element.firstChildElement("connector");
	int i = 0;
	while (!connectorElement.isNull()) {
		Connector *connector = m_connectors[i++];
        qreal angle = connectorElement.attribute("angle").toDouble();
		connector->setAngle(angle);

        qreal x = connectorElement.attribute("x").toDouble();
        qreal y = connectorElement.attribute("y").toDouble();
        connector->setPos(QPointF(x, y));
        if (connectorElement.hasAttribute("hub_owner")) {
            DiagramObject *owner = qobject_cast<DiagramObject *>(document->itemById(QUuid(connectorElement.attribute("hub_owner"))));
            connector->setHub(owner->hub());
        }
        connectorElement = connectorElement.nextSiblingElement("connector");
    }
}

void Line::saveToXml(QDomElement element, QDomDocument doc)
{
    DiagramItem::saveToXml(element,doc);
	for (int i = 0; i < 2; i++) {
		Connector *connector = m_connectors[i];
		QDomElement connectorElement = doc.createElement("connector");
        connectorElement.setAttribute("x", connector->pos().x() );
        connectorElement.setAttribute("y", connector->pos().y() );
        connectorElement.setAttribute("angle", connector->angle() );
        element.appendChild(connectorElement);
		if (connector->hub()) {
            connectorElement.setAttribute("hub_owner", connector->hub()->owner()->id().toString());
		}
    }
}

QPolygonF Line::linePoints() const
{
	return m_linePoints;
}

void Line::setLinePoints(const QPolygonF& points)
{
	if (m_linePoints != points) {
		prepareGeometryChange();
		m_linePoints = points;
		m_dirty = true;
	}
}

Qt::PenStyle Line::lineStyle() const
{
	return m_lineStyle;
}

void Line::setLineStyle(Qt::PenStyle style)
{
	if (m_lineStyle != style) {
		// prepareGeometryChange();
		m_lineStyle = style;
		// m_dirty = true;
	}
}

QPainterPath Line::startArrow() const
{
	return m_startArrow;
}

void Line::setStartArrow(const QPainterPath& path)
{
	if (m_startArrow != path) {
		prepareGeometryChange();
		m_startArrow = path;
		m_dirty = true;
	}
}

bool Line::fillStartArrow() const
{
	return m_fillStartArrow;
}

void Line::setFillStartArrow(bool fill)
{
	if (m_fillStartArrow != fill) {
		m_fillStartArrow = fill;
		m_dirty = true;
	}
}

QPainterPath Line::endArrow() const
{
	return m_endArrow;
}

void Line::setEndArrow(const QPainterPath& path)
{
	if (m_endArrow != path) {
		prepareGeometryChange();
		m_endArrow = path;
		m_dirty = true;
	}
}

bool Line::fillEndArrow() const
{
	return m_fillEndArrow;
}

void Line::setFillEndArrow(bool fill)
{
	if (m_fillEndArrow != fill) {
		m_fillEndArrow = fill;
		m_dirty = true;
	}
}

void Line::updateCache()
{
	m_dirty = false;

	QPainterPath path;
	if (m_linePoints.isEmpty()) {
		// Nothing to process
		m_shape = path;
		return;
	}

	if (!m_startArrow.isEmpty()) {
		QLineF firstSegment(m_linePoints.at(0), m_linePoints.at(1));
        QTransform transform;
        transform.translate(firstSegment.p1().x(), firstSegment.p1().y());
        transform.rotate(360 - firstSegment.angle() + 90 + 180);
        m_transformedStartArrow = transform.map(m_startArrow);
    } else {
		m_transformedStartArrow = QPainterPath();
	}

	if (!m_endArrow.isEmpty()) {
		int size = m_linePoints.size();
		QLineF lastSegment(m_linePoints.at(size-2), m_linePoints.at(size-1));
        QTransform matrix;
		matrix.translate(lastSegment.p2().x(), lastSegment.p2().y());
		matrix.rotate(-lastSegment.angle() + 90);
		m_transformedEndArrow = matrix.map(m_endArrow);
    } else {
		m_transformedEndArrow = QPainterPath();
	}

	// Add the main line
	path.moveTo(m_linePoints.at(0));
	for (int i = 1; i < m_linePoints.size(); i++)
		path.lineTo(m_linePoints.at(i));

	path.addPath(m_transformedStartArrow);
	path.addPath(m_transformedEndArrow);

	// Add an outline around the path
	QPainterPathStroker ps;
	ps.setWidth(4.33);
	ps.setJoinStyle(Qt::MiterJoin);
	m_shape = ps.createStroke(path);
}

QRectF Line::boundingRect() const
{
	return shape().controlPointRect();
}

QPainterPath Line::shape() const
{
	if (m_dirty)
		const_cast<Line *>(this)->updateCache();

	return m_shape;
}

void Line::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	if (m_dirty)
		updateCache();

	QPen pen(QPen(QColor(0, 0, 0), 1.33));
	pen.setJoinStyle(Qt::MiterJoin);
	pen.setStyle(m_lineStyle);
    if (isSelected()) {
		pen.setColor(QColor(0, 96, 255));
    }

	painter->setPen(pen);
	painter->drawPolyline(m_linePoints);

	pen.setStyle(Qt::SolidLine);
	painter->setPen(pen);
	if (m_fillStartArrow) {
		painter->setBrush(pen.color());
	}
	painter->drawPath(m_transformedStartArrow);
	if (m_fillEndArrow) {
		if (!m_fillStartArrow) {
			painter->setBrush(pen.color());
		}
    } else {
		if (m_fillStartArrow) {
			painter->setBrush(QBrush());
		}
	}
	painter->drawPath(m_transformedEndArrow);
}
