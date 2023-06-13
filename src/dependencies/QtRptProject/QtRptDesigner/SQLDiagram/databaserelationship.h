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

#ifndef DATABASERELATION_H
#define DATABASERELATION_H

#include <QPair>
#include <QGraphicsItem>
#include <QPainter>
#include "line.h"
#include "databasetable.h"
#include "connector.h"

class DatabaseRelationship : public Line
{
	Q_OBJECT
    /*
    Q_PROPERTY(Cardinality cardinality READ cardinality WRITE setCardinality);
	Q_PROPERTY(bool childOptional READ isChildOptional WRITE setChildOptional);
	Q_PROPERTY(bool parentOptional READ isParentOptional WRITE setParentOptional);
	Q_PROPERTY(Column* childColumn READ childColumn WRITE setChildColumn);
    Q_PROPERTY(Column* parentColumn READ parentColumn WRITE setParentColumn);*/

public:
	DatabaseRelationship(DiagramItem *parent = 0);
	~DatabaseRelationship();
	
	DatabaseTable *childTable() const { return qobject_cast<DatabaseTable *>(connector(0)->connectedObject()); }
	DatabaseTable *parentTable() const { return qobject_cast<DatabaseTable *>(connector(1)->connectedObject()); }

	enum { Type = DiagramItem::Relation };
	virtual int type() const { return Type; }

    static const char *staticTypeName() { return "database-relationship"; }
    virtual const char *typeName() { return staticTypeName(); }

	void loadFromXml(QDomElement element, DiagramDocument *document = 0);
    void saveToXml(QDomElement element, QDomDocument doc);

	//! Returns true if the foreign key on the child table is also the primary key
	bool isIdentifying() const;

	//! List of columns in the child (referencing) table
	Column *childColumn() const;
	void setChildColumn(Column *column);

	//! List of columns in the parent (referenced) table
	Column *parentColumn() const;
	void setParentColumn(Column *column);

	enum Action {
		NoAction,
		Restrict,
		Cascade,
		SetNull,
		SetDefault
	};

	Action onUpdateAction() const;
	Action onDeleteAction() const;

public slots:
	void updateLayout();
	void updateEnds();

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
	class PrivateData;
	PrivateData *const d;
};

#endif
