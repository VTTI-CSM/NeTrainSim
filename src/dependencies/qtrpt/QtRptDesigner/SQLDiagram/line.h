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

#ifndef DIAGRAMCONNECTION_H
#define DIAGRAMCONNECTION_H

#include <QPolygonF>
#include <QPainterPath>
#include "diagramitem.h"
#include "diagramdocument.h"

class Connector;
class DiagramObject;


class Line : public DiagramItem
{
	Q_OBJECT

public:
    explicit Line(DiagramItem *parent = 0);
	~Line();

    /*enum LineLayout
	{
		StraightLineLayout,
		OrthogonalLineLayout
    };*/

	Connector *connector(int index) const;

	QRectF boundingRect() const;
	QPainterPath shape() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

	virtual void updateLayout();

	virtual void loadFromXml(QDomElement element, DiagramDocument *document = 0);
    void saveToXml(QDomElement element, QDomDocument doc);

	//! The path of the line
	QPolygonF linePoints() const;

	//! Set the path of the line
	void setLinePoints(const QPolygonF &points);

	//! The style of the line
	Qt::PenStyle lineStyle() const;

	//! Set the style of the line
	void setLineStyle(Qt::PenStyle style);

	//! Path of the start arrow
	QPainterPath startArrow() const;

	//! Set the path of the start arrow
	void setStartArrow(const QPainterPath &path);

	//! Should the start arrow be filled?
	bool fillStartArrow() const;

	//! Set whether the start arrow should be filled.
	void setFillStartArrow(bool fill);

	//! Path of the end arrow
	QPainterPath endArrow() const;

	//! Set the path of the end arrow
	void setEndArrow(const QPainterPath &path);

	//! Should the end arrow be filled?
	bool fillEndArrow() const;

	//! Set whether the end arrow should be filled.
	void setFillEndArrow(bool fill);

private:
	void updateCache();

	Connector *m_connectors[2];
	QPolygonF m_linePoints;
	Qt::PenStyle m_lineStyle;
	QPainterPath m_shape;
	QPainterPath m_startArrow;
	QPainterPath m_transformedStartArrow;
	bool m_fillStartArrow;
	QPainterPath m_endArrow;
	QPainterPath m_transformedEndArrow;
	bool m_fillEndArrow;
	bool m_dirty;
};

#endif
