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

#include <QDebug>
#include <QGraphicsItem>
#include <QPair>
#include "connector.h"
#include "diagramobject.h"
#include "line.h"
#include "boxsidehub.h"

BoxSideHub::BoxSideHub(DiagramObject *owner)
: Hub(owner)
{}

enum Side {
	Top,
	Right,
	Bottom,
	Left
};

static int cmpAngle(qreal a, qreal b)
{
	qreal d = a - b;
	if (d > 180)
		d = d - 360;
	else if (d < -180)
		d = d + 360;
	return d;
}

typedef QPair<Connector *, qreal> ConnectorRealPair;

static bool itemLessThan(const ConnectorRealPair &a, const ConnectorRealPair &b)
{
	int r = cmpAngle(a.second, b.second);
	if (r == 0) {
        //return a.first->owner() < b.first->owner();
	}
	return r < 0;
}

//              |
//    180-cA    |   cA
//    _____________________
//              |
//    180+cA    |   360-cA
//              |

void BoxSideHub::update()
{
	QHash<Side, QList<ConnectorRealPair> > sides;
    DiagramObject *item = owner();
	QRectF rect = item->boundingRect().translated(item->pos());

    for (auto &connector : connectors()) {
		Line *connection = connector->owner();
		Connector *connector1 = connection->connector(0);
		Connector *connector2 = connection->connector(1);
        DiagramObject *item2;
		if (connector1 == connector) {
    		item2 = connector2->connectedObject();
		}
		else {
            item2 = connector1->connectedObject();
		}
		QRectF rect1 = rect;
		QRectF rect2 = item2->boundingRect().translated(item2->pos());
		qreal angle = QLineF(rect1.center(), rect2.center()).angle();
		qreal cornerAngle = QLineF(rect1.center(), rect1.topRight()).angle();
		Side side = Right;
		if (angle > cornerAngle) {
			if (angle <= 180 - cornerAngle) {
				side = Top;
			}
			else if (angle <= 180 + cornerAngle) {
				side = Left;
			}
			else if (angle <= 360 - cornerAngle) {
				side = Bottom;
			}
		}
		sides[side].append(qMakePair(connector, angle));
    }

    for (auto &side : sides.keys()) {
		QPointF p, dp;
		QList<ConnectorRealPair> c = sides[side];
        std::sort(c.begin(), c.end(), itemLessThan);
		qreal angle;
		switch (side) {
		case Top:
			p = rect.topRight();
			dp = QPointF(-rect.width() / (c.size() + 1), 0);
			angle = 90;
			break;
		case Right:
			p = rect.bottomRight();
			dp = QPointF(0, -rect.height() / (c.size() + 1));
			angle = 0;
			break;
		case Bottom:
			p = rect.bottomLeft();
			dp = QPointF(rect.width() / (c.size() + 1), 0);
			angle = 270;
			break;
		case Left:
			p = rect.topLeft();
			dp = QPointF(0, rect.height() / (c.size() + 1));
			angle = 180;
			break;
		}
        for (auto &item : c) {
			p += dp;
			item.first->setAngle(angle);
			item.first->setPos(p);
        }
	}
}
