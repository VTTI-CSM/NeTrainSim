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

#ifndef COLUMNLIST_H
#define COLUMNLIST_H

#include <QAbstractTableModel>
#include "databasetable.h"

class Column;

class ColumnList : public QObject
{
	Q_OBJECT

public:
	ColumnList(DatabaseTable *table);

	DatabaseTable *table() const { return qobject_cast<DatabaseTable *>(parent()); }

	QList<Column *> columns() { return m_columns; }
	int columnCount() const { return m_columns.size(); }
	int indexOf(Column *column) const { return m_columns.indexOf(column); }
	Column *column(int index) { return m_columns[index]; }
	int insertColumn(int, Column *);
	int appendColumn(Column *);
	void swapColumns(int, int);
	int removeColumn(Column *);
	Column *removeColumn(int);

signals:
	void columnAboutToBeInserted(int index);
	void columnInserted(int index);
	void columnAboutToBeRemoved(int index);
	void columnRemoved(int index);
	void columnChanged(int index);

protected slots:
	void updateColumn(const QString &name, const QVariant&);

private:
	QList<Column *> m_columns;
};

#endif
