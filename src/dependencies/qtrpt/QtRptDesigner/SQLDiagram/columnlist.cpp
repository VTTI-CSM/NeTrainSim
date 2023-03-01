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
#include "columnlist.h"
#include "column.h"
#include "databasetable.h"

ColumnList::ColumnList(DatabaseTable *table)
: QObject(table)
{}

int ColumnList::insertColumn(int index, Column *column)
{
	Q_ASSERT(m_columns.contains(column) == false);
	column->setParent(this);
	connect(column, SIGNAL(propertyChanged(const QString &, const QVariant &)), this, SLOT(updateColumn(const QString &, const QVariant &)));
	emit columnAboutToBeInserted(index);
	m_columns.insert(index, column);
	emit columnInserted(index);
	return index;
}

int ColumnList::appendColumn(Column *column)
{
	return insertColumn(columnCount(), column);
}

int ColumnList::removeColumn(Column *column)
{
	int index = m_columns.indexOf(column);
	removeColumn(index);
	return index;
}

Column *ColumnList::removeColumn(int index)
{
	emit columnAboutToBeRemoved(index);
	Column *column = m_columns.takeAt(index);
	emit columnRemoved(index);
	disconnect(column, 0, this, 0);
	column->setParent(0);
	return column;
}

void ColumnList::swapColumns(int oldIndex, int newIndex)
{
	m_columns.move(oldIndex, newIndex);
	emit columnChanged(oldIndex);
	emit columnChanged(newIndex);
}

void ColumnList::updateColumn(const QString &, const QVariant &)
{
	Column *column = qobject_cast<Column *>(sender());
	int index = m_columns.indexOf(column);
	Q_ASSERT(index != -1);
	emit columnChanged(index);
}
