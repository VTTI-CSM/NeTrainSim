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

#ifndef DATABASEMODELITEM_H
#define DATABASEMODELITEM_H

#include <QGraphicsItem>
#include <QDomDocument>
#include <QDomElement>
#include <QUuid>

class QMimeData;
class DiagramDocument;
class DiagramItemProperties;

class DiagramItem : public QObject, public QGraphicsItem
{
	Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
	enum {
		Table = UserType + 1,
		Relation
	};

	DiagramDocument *document() const;

	DiagramItem(DiagramItem *parent = 0);
	virtual ~DiagramItem();

    static const char *staticTypeName() { return ""; }
    virtual const char *typeName() = 0;

    virtual void loadFromXml(QDomElement element, DiagramDocument *document = 0);
    virtual void saveToXml(QDomElement element, QDomDocument doc);
/*
	virtual QMimeData *toMimeData();
    static DiagramItem *fromMimeData(const QMimeData *mimeData);*/

    QUuid id() const { return m_id; }
    void setId(QUuid id);
    void createId();

private:
    QUuid m_id;

signals:
    void propertyChanged(const QString &name, const QVariant &value);
};

#endif
