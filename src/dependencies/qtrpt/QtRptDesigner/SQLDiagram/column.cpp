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

#include "column.h"
#include "columnlist.h"
#include "hub.h"
#include "databaserelationship.h"

Column::Column(ColumnList *columnList)
: QObject(columnList), m_primaryKey(false), m_required(false)
{}

void Column::setName(const QString &name)
{
	if (m_name != name) {
		m_name = name;
		emit propertyChanged("name", name);
	}
}

void Column::setDataType(const QString &dataType)
{
	if (m_dataType != dataType) {
		m_dataType = dataType;
		emit propertyChanged("dataType", dataType);
	}
}

void Column::setPrimaryKey(bool primaryKey)
{
	if (m_primaryKey != primaryKey) {
		m_primaryKey = primaryKey;
		emit propertyChanged("primaryKey", primaryKey);
	}
}

void Column::setRequired(bool required)
{
	if (m_required != required) {
		m_required = required;
		emit propertyChanged("required", required);
	}
}

bool Column::isForeignKey() const
{
    for (Line *line : columnList()->table()->hub()->outgoingLines()) {
		DatabaseRelationship *rel = (DatabaseRelationship *)line;
		if (rel->childColumn() == this) {
			return true;
		}
	}
	return false;
}
