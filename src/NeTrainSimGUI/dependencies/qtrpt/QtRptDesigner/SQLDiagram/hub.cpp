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

#include "hub.h"
#include "diagramobject.h"
#include "diagramdocument.h"
#include "connector.h"
#include "line.h"

Hub::Hub(DiagramObject *owner)
: m_owner(owner)
{}

DiagramObject *Hub::owner() const
{
	return m_owner;
}

void Hub::addConnector(Connector *connector)
{
	m_connectors.insert(connector);
    if (m_owner->document())
		m_owner->document()->updatePositions(m_owner);
}

void Hub::removeConnector(Connector *connector)
{
	m_connectors.remove(connector);
    if (m_owner->document())
		m_owner->document()->updatePositions(m_owner);
}

QSet<Connector *> Hub::connectors() const
{
	return m_connectors;
}

void Hub::update()
{}

QList<Line *> Hub::outgoingLines() const
{
    QList<Line *> result;
    for (Connector *connector : m_connectors) {
		Line *line = connector->owner();
		if (connector == line->connector(0)) {
			result << line;
		}
	}
	return result;
}

QList<Line *> Hub::incomingLines() const
{
    QList<Line *> result;
    for (Connector *connector : m_connectors) {
        Line *line = connector->owner();
        if (connector == line->connector(1)) {
            result << line;
        }
    }
    return result;
}
