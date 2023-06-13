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

#include <QGraphicsScene>
#include <QDebug>
#include "diagramdocument.h"
#include "column.h"
#include "columnlist.h"
#include "databasetable.h"
#include "line.h"
#include "databaserelationship.h"
#include "hub.h"

class DatabaseRelationship::PrivateData
{
public:
	PrivateData() :
		onUpdateAction(NoAction),
		onDeleteAction(NoAction),
        childColumn(nullptr),
        parentColumn(nullptr)
{
		if (!pathsInitialized) {
			pathsInitialized = true;

			paths[1].moveTo(-5, 0);
			paths[1].lineTo(0, 8);
			paths[1].lineTo(5, 0);
			paths[1].moveTo(-4, 10);
			paths[1].lineTo(4, 10);

			paths[3].moveTo(-4, 5);
			paths[3].lineTo(4, 5);
			paths[3].moveTo(-4, 8);
			paths[3].lineTo(4, 8);

			paths[2].moveTo(-4, 5);
			paths[2].lineTo(4, 5);
			paths[2].addEllipse(-3.5, 8, 7, 7);

			paths[0].moveTo(-5, 0);
			paths[0].lineTo(0, 8);
			paths[0].lineTo(5, 0);
			paths[0].addEllipse(-3.5, 8, 7, 7);

			paths[4].moveTo(-4.5, 9);
			paths[4].lineTo(0, 0);
			paths[4].lineTo(4.5, 9);
			paths[4].lineTo(-4.5, 9);
		}
	}

	Action onUpdateAction;
	Action onDeleteAction;
	Column *childColumn;
	Column *parentColumn;

    /*QPainterPath crowsFootPath(bool toMany, bool optional) {
		if (toMany) {
			if (optional) // 0..*
				return paths[0];
			else // 1..*
				return paths[1];
        } else {
			if (optional) // 0..1
				return paths[2];
			else // 1..1
				return paths[3];
		}
    }*/

    QPainterPath arrowHeadPath() {
		return paths[4];
	}

	static bool pathsInitialized;
	static QPainterPath paths[5];
};

bool DatabaseRelationship::PrivateData::pathsInitialized = false;
QPainterPath DatabaseRelationship::PrivateData::paths[5];

DatabaseRelationship::DatabaseRelationship(DiagramItem *parent) : Line(parent), d(new PrivateData)
{
	setFlag(ItemIsSelectable);
}

DatabaseRelationship::~DatabaseRelationship()
{
	delete d;
}

Column *DatabaseRelationship::childColumn() const
{
	return d->childColumn;
}

void DatabaseRelationship::setChildColumn(Column *column)
{
	if (d->childColumn != column) {
		d->childColumn = column;
        emit propertyChanged("childColumn", QVariant::fromValue<Column*>(column));
		static_cast<DatabaseTable *>(connector(0)->hub()->owner())->updateLayout();
		updateLayout();
		update();
	}
}

Column *DatabaseRelationship::parentColumn() const
{
	return d->parentColumn;
}

void DatabaseRelationship::setParentColumn(Column *column)
{
	if (d->parentColumn != column) {
		d->parentColumn = column;
        emit propertyChanged("parentColumn", QVariant::fromValue<Column*>(column));
		updateLayout();
		update();
	}
}

bool DatabaseRelationship::isIdentifying() const
{
	DatabaseTable *table = childTable();
	if (table) {
		Column *column = childColumn();
		if (column) {
			return column->isPrimaryKey();
		}
	}
	return false;
}

QVariant DatabaseRelationship::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSceneHasChanged)
		updateLayout();

	return QGraphicsItem::itemChange(change, value);
}

void DatabaseRelationship::updateEnds()
{
	if (!document())
		return;

    setFillStartArrow(false);
    setFillEndArrow(true);
    setStartArrow(QPainterPath());
    setEndArrow(d->arrowHeadPath());
    setLineStyle(Qt::SolidLine);
}

void DatabaseRelationship::updateLayout()
{
	prepareGeometryChange();

    Connector *connector1 = connector(0);
    Connector *connector2 = connector(1);

	bool haveLine = false;

	QPolygonF line;

	// Orthogonal line
	if (!haveLine) {
		QPointF p1 = connector1->pos();
		QPointF p2 = connector2->pos();
		qreal a1 = connector1->angle();
		qreal a2 = connector2->angle();

		QLineF line1 = QLineF::fromPolar(1, a1).translated(p1);
		QLineF line2 = QLineF::fromPolar(1, a2).translated(p2);
		QPointF intersection;
		line << p1;

        #if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
            auto interSect = line1.intersects(line2, &intersection);
        #else
            auto interSect = line1.intersect(line2, &intersection);
        #endif

        if (interSect != QLineF::NoIntersection) {
			// 2-segment line
			line << intersection;
		}
		else {
            #if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
                auto interSect = line1.intersects(line2.normalVector(), &intersection);
            #else
                auto interSect = line1.intersect(line2.normalVector(), &intersection);
            #endif

            if (interSect != QLineF::NoIntersection) {
				// 3-segment line
				qreal len = QLineF(p1, intersection).length() * 0.5;
				line << QLineF::fromPolar(len, a1).translated(p1).p2();
				line << QLineF::fromPolar(len, a2).translated(p2).p2();
			}
			else {
				qFatal("No line?");
			}
		}
		line << p2;
		haveLine = true;
	}

	// Simple center<->center line
	if (!haveLine) {
		QPointF p1 = connector1->pos();
		QPointF p2 = connector2->pos();
		line << p1 << p2;
		haveLine = true;
	}

	setLinePoints(line);
	updateEnds();
}

void DatabaseRelationship::loadFromXml(QDomElement element, DiagramDocument *document)
{
    Line::loadFromXml(element, document);

    if (element.hasAttribute("columns_child") &&
        element.attribute("columns_child","-1").toInt() != -1 && childTable() ) {
        setChildColumn(childTable()->columnList()->column(element.attribute("columns_child","-1").toInt()));
    }
    if (element.hasAttribute("columns_parent") &&
        element.attribute("columns_parent","-1").toInt() != -1 && childTable() ) {
        setParentColumn(parentTable()->columnList()->column(element.attribute("columns_parent","-1").toInt()));
    }
}

void DatabaseRelationship::saveToXml(QDomElement element, QDomDocument doc)
{
    Line::saveToXml(element,doc);

    if (d->childColumn)
        element.setAttribute("columns_child", childTable()->columnList()->indexOf(d->childColumn) );
    if (d->parentColumn)
        element.setAttribute("columns_parent", parentTable()->columnList()->indexOf(d->parentColumn) );
}
