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

#ifndef DATABASETABLE_H
#define DATABASETABLE_H

#include <QGraphicsItem>
#include <QPainter>
#include <QList>
#include "diagramobject.h"

class Column;
class ColumnList;
class DiagramDocument;

class DatabaseTable : public DiagramObject
{
	Q_OBJECT
    //Q_PROPERTY(QString name READ name WRITE setName)
    //Q_PROPERTY(QColor color READ color WRITE setColor)

public:
	DatabaseTable(DiagramItem *parent = 0);

	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

	QString name() const { return m_name; }
    void setName(const QString &name);
    void setInitialName(int counter);
	QColor color() const { return m_color; }
    void setColor(const QColor &color);
    QList<Column *> primaryKeys() const;
    ColumnList *columnList() const { return m_columnList; }

//// 	void setColumn(int index, const QString &name);
//// 	void removeColumn(int index);
//// 	Column *addColumn(const QString &name = QString());
//// 	void swapColumns(int index1, int index2);

	enum { Type = DiagramItem::Table };
	virtual int type() const { return Type; }

	static const char *staticTypeName() { return "database-table"; }
	virtual const char *typeName() { return staticTypeName(); }

    void loadFromXml(QDomElement element, DiagramDocument *document);
    void saveToXml(QDomElement element, QDomDocument doc);

//// 	QMimeData *toMimeData();

    void setColumns(ColumnList *columnList);
    void getSelectedColumns(QString &value);

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
	QString m_name;
	QColor m_color;
    ColumnList *m_columnList;
	QList<QString> m_columnLabels;
	
	QRectF m_outerRect, m_nameBgRect;
	QPointF m_namePos, m_firstColPos, m_colPosIncrement;
	qreal m_leftSideWidth;

public slots:
    void updateLayout();
};

#endif
