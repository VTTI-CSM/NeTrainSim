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

#include <QMimeData>
#include <QDomProcessingInstruction>
#include <QDebug>
#include "diagramdocument.h"
#include "diagramitem.h"
//#include "domutils.h"

DiagramItem::DiagramItem(DiagramItem *parent)
: QGraphicsItem(parent)
{}

DiagramItem::~DiagramItem()
{}

DiagramDocument *DiagramItem::document() const
{
	return qobject_cast<DiagramDocument *>(scene());
}

void DiagramItem::loadFromXml(QDomElement element, DiagramDocument *)
{
    setId(QUuid(element.attribute("id", QUuid().toString())));
    qreal x = element.attribute("x").toDouble();
    qreal y = element.attribute("y").toDouble();
    setPos(QPointF(x, y));
}

void DiagramItem::saveToXml(QDomElement element, QDomDocument doc)
{
    Q_UNUSED(doc);
    element.setAttribute("type", typeName());
    if (id() != QUuid()) {
        element.setAttribute("id", id().toString());
        element.setAttribute("x", pos().x());
        element.setAttribute("y", pos().y());
    }
}

void DiagramItem::setId(QUuid id)
{
    m_id = id;
}

void DiagramItem::createId()
{
    m_id = QUuid::createUuid();
}

/*
QMimeData *DiagramItem::toMimeData() {
	QDomDocument doc;
	QDomProcessingInstruction xml = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(xml);
	QDomElement element = doc.createElement("item");
	doc.appendChild(element);
	saveToXml(doc, element);

	QByteArray data;
	QTextStream stream(&data);
	doc.save(stream, 0);

	QMimeData *mimeData = new QMimeData();
	mimeData->setData("application/dbmodel.item", data);
	return mimeData;
}

DiagramItem *DiagramItem::fromMimeData(const QMimeData *mimeData) {
	QByteArray data = mimeData->data("application/dbmodel.item");
	if (data.isNull())
		return 0;

	QDomDocument doc;
	doc.setContent(data);

	QDomElement element = doc.firstChildElement("item");
	if (element.isNull())
		return 0;

	DiagramItem *item = DiagramItemFactory::create(element.attribute("type"));
    if (item == nullptr)
		return 0;

	item->loadFromXml(element);
	item->createId();
	return item;
}
*/
