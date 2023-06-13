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
#include "commands.h"
#include "line.h"
#include "hub.h"
#include "diagramdocument.h"
#include "diagramitem.h"
#include "diagramobject.h"
#include "connector.h"

SetObjectPropertyCommand::SetObjectPropertyCommand(QObject *object, const char *name, const QVariant &value, QUndoCommand *parent)
: QUndoCommand(parent), m_object(object), m_name(name), m_newValue(value)
{
	m_oldValue = object->property(name);
}

void SetObjectPropertyCommand::redo()
{
	m_object->setProperty(m_name.latin1(), m_newValue);
}

void SetObjectPropertyCommand::undo()
{
	m_object->setProperty(m_name.latin1(), m_oldValue);
}

int SetObjectPropertyCommand::id() const
{
	return COMMAND_SET_OBJECT_PROPERTY;
}

bool SetObjectPropertyCommand::mergeWith(const QUndoCommand *o)
{
	return false;
	Q_ASSERT(id() == o->id());
	const SetObjectPropertyCommand *other = static_cast<const SetObjectPropertyCommand *>(o);
	if (m_object != other->m_object || m_name != other->m_name)
		return false;
	m_newValue = other->m_newValue;
	return true;
}


MoveItemCommand::MoveItemCommand(DiagramItem *item, const QPointF &oldPos, const QPointF &newPos, QUndoCommand *parent)
: QUndoCommand(parent), m_item(item), m_oldPos(oldPos), m_newPos(newPos)
{}

void MoveItemCommand::redo()
{
	m_item->setPos(m_newPos);
}

void MoveItemCommand::undo()
{
	m_item->setPos(m_oldPos);
}


AddObjectCommand::AddObjectCommand(DiagramDocument *document, DiagramObject *item, QUndoCommand *parent)
: QUndoCommand(parent), m_document(document), m_item(item), m_owner(true)
{}

AddObjectCommand::~AddObjectCommand()
{
	if (m_owner)
		delete m_item;
}

void AddObjectCommand::redo()
{
	m_document->addItem(m_item);
	m_owner = false;
}

void AddObjectCommand::undo()
{
	m_document->removeItem(m_item);
	m_owner = true;
}

RemoveObjectCommand::RemoveObjectCommand(DiagramDocument *document, DiagramObject *object, QUndoCommand *parent)
: QUndoCommand(parent), m_document(document), m_object(object), m_owner(false)
{
    for (Line *line : document->findConnections(object))
		new RemoveLineCommand(document, line, this);
}

RemoveObjectCommand::~RemoveObjectCommand()
{
    if (m_owner)
		delete m_object;
}

void RemoveObjectCommand::redo()
{
	QUndoCommand::redo();
	m_document->removeItem(m_object);
	m_owner = true;
}

void RemoveObjectCommand::undo()
{
	m_document->addItem(m_object);
	m_owner = false;
	QUndoCommand::undo();
}

RemoveLineCommand::RemoveLineCommand(DiagramDocument *document, Line *line, QUndoCommand *parent)
: QUndoCommand(parent), m_document(document), m_line(line), m_owner(false)
{}

RemoveLineCommand::~RemoveLineCommand()
{
    if (m_owner)
		delete m_line;
}

void RemoveLineCommand::redo()
{
	m_line->connector(0)->removeFromHub();
	m_line->connector(1)->removeFromHub();
	m_document->removeItemLater(m_line);
	m_owner = true;
}

void RemoveLineCommand::undo()
{
	m_line->connector(0)->addToHub();
	m_line->connector(1)->addToHub();
	m_document->addItemLater(m_line);
	m_owner = false;
}

AddLineCommand::AddLineCommand(DiagramDocument *document, Line *line, QUndoCommand *parent)
: QUndoCommand(parent), m_document(document), m_line(line), m_owner(true)
{
	m_line->connector(0)->removeFromHub();
	m_line->connector(1)->removeFromHub();
}

AddLineCommand::~AddLineCommand()
{
    if (m_owner)
		delete m_line;
}

void AddLineCommand::redo()
{
	m_line->connector(0)->addToHub();
	m_line->connector(1)->addToHub();
	m_document->addItemLater(m_line);
	m_owner = false;
}

void AddLineCommand::undo()
{
	m_line->connector(0)->removeFromHub();
	m_line->connector(1)->removeFromHub();
	m_document->removeItemLater(m_line);
	m_owner = true;
}
