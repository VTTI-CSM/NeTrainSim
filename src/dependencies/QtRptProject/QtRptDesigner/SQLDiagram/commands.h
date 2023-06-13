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

#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include <QVariant>
#include <QPointer>
#include <QPointF>
#include <QLatin1String>

class Line;
class DiagramDocument;
class DiagramItem;
class DiagramObject;

enum {
	COMMAND_SET_OBJECT_PROPERTY,
};

class SetObjectPropertyCommand : public QUndoCommand
{
public:
	SetObjectPropertyCommand(QObject *object, const char *name, const QVariant &value, QUndoCommand *parent = 0);
	void undo();
	void redo();
	int id() const;
	bool mergeWith(const QUndoCommand *command);

private:
	QObject *m_object;
	QLatin1String m_name;
	QVariant m_oldValue, m_newValue;
};

class MoveItemCommand : public QUndoCommand
{
public:
	MoveItemCommand(DiagramItem *item, const QPointF &oldPos, const QPointF &newPos, QUndoCommand *parent = 0);
	void undo();
	void redo();

private:
	QPointer<DiagramItem> m_item;
	QPointF m_oldPos, m_newPos;
};

class AddObjectCommand : public QUndoCommand
{
public:
	AddObjectCommand(DiagramDocument *document, DiagramObject *item, QUndoCommand *parent = 0);
	~AddObjectCommand();
	void undo();
	void redo();
private:
	DiagramDocument *m_document;
	DiagramObject *m_item;
	bool m_owner;
};

class RemoveObjectCommand : public QUndoCommand
{
public:
	RemoveObjectCommand(DiagramDocument *document, DiagramObject *object, QUndoCommand *parent = 0);
	~RemoveObjectCommand();
	void undo();
	void redo();
protected:
	DiagramDocument *m_document;
	DiagramObject *m_object;
	bool m_owner;
};

class RemoveLineCommand : public QUndoCommand
{
public:
	RemoveLineCommand(DiagramDocument *document, Line *line, QUndoCommand *parent = 0);
	~RemoveLineCommand();
	void undo();
	void redo();
protected:
	DiagramDocument *m_document;
	Line *m_line;
	bool m_owner;
};

class AddLineCommand : public QUndoCommand
{
public:
	AddLineCommand(DiagramDocument *document, Line *line, QUndoCommand *parent = 0);
	~AddLineCommand();
	void undo();
	void redo();
private:
	DiagramDocument *m_document;
	Line *m_line;
	bool m_owner;
};

#endif
