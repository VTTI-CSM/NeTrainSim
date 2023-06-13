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

#ifndef HUB_H
#define HUB_H

#include <QSet>

class Connector;
class Line;
class DiagramObject;

class Hub
{
public:
	Hub(DiagramObject *owner);
	// Virtual destructor provided to reduce compiler warnings
	virtual ~Hub() {};
	DiagramObject *owner() const;

    void addConnector(Connector *connector);
    void removeConnector(Connector *connector);
    virtual void update();
    QSet<Connector *> connectors() const;
    QList<Line *> outgoingLines() const;
    QList<Line *> incomingLines() const;

private:
    QSet<Connector *> m_connectors;
	DiagramObject *m_owner;
};

#endif
