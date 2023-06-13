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

#ifndef COLUMN_H
#define COLUMN_H

#include <QObject>
#include <QString>
#include "columnlist.h"

class Column : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString name READ name WRITE setName)
	Q_PROPERTY(QString dataType READ dataType WRITE setDataType)
	Q_PROPERTY(bool primaryKey READ isPrimaryKey WRITE setPrimaryKey)
	Q_PROPERTY(bool required READ isRequired WRITE setRequired)

public:

	Column(ColumnList *columnList = 0);

	ColumnList *columnList() const { return qobject_cast<ColumnList *>(parent()); }

	QString name() const { return m_name; }
	void setName(const QString &name);

	QString dataType() const { return m_dataType; }
	void setDataType(const QString &dataType);

	bool isPrimaryKey() const { return m_primaryKey; }
	void setPrimaryKey(bool primaryKey);

	bool isForeignKey() const;

    bool isRequired() const { return m_required; } //The column selected for query
	void setRequired(bool required);

signals:
	void propertyChanged(const QString &name, const QVariant &value);

private:
	QString m_name;
	QString m_dataType;
	bool m_primaryKey;
	bool m_required;
};

Q_DECLARE_METATYPE(Column*)

#endif
