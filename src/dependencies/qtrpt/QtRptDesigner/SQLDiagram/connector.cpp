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

#include "connector.h"
#include "diagramdocument.h"
#include "hub.h"
#include "line.h"

Connector::Connector(Line *owner)
: m_owner(owner), m_hub(nullptr)
{}

Line *Connector::owner() const
{
	return m_owner;
}

QPointF Connector::pos() const
{
	return m_pos;
}

void Connector::setPos(const QPointF &pos)
{
	if (m_pos != pos) {
		m_pos = pos;
		if (m_owner->document())
			m_owner->document()->updateLineLayout(m_owner);
	}
}

qreal Connector::angle() const
{
	return m_angle;
}

void Connector::setAngle(qreal angle)
{
	if (m_angle != angle) {
		m_angle = angle;
		if (m_owner->document())
			m_owner->document()->updateLineLayout(m_owner);
	}
}

Hub *Connector::hub() const
{
	return m_hub;
}

void Connector::setHub(Hub *hub)
{
	removeFromHub();
	m_hub = hub;
	addToHub();
}

bool Connector::isConnected() const
{
	return m_hub;
}

DiagramObject *Connector::connectedObject() const
{
    if (isConnected())
        return hub()->owner();
    return nullptr;
}

Connector *Connector::otherEnd() const
{
	Line *conn = owner();
    if (conn->connector(0) == this)
		return conn->connector(1);
    else
		return conn->connector(0);
}

void Connector::addToHub()
{
	if (m_hub)
		m_hub->addConnector(this);
}

void Connector::removeFromHub()
{
	if (m_hub)
		m_hub->removeConnector(this);
}
