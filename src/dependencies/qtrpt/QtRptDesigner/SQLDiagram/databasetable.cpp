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

#include <QGraphicsScene>
#include <QStringList>
#include <QDebug>
#include "databasetable.h"
#include "diagramdocument.h"
#include "column.h"
#include "columnlist.h"
#include "boxsidehub.h"

DatabaseTable::DatabaseTable(DiagramItem *parent)
: DiagramObject(parent)
{
	setFlag(ItemIsMovable);
	setFlag(ItemIsSelectable);
#if QT_VERSION >= 0x040600
	// This flag was introduced in Qt 4.6.0
	// see Assistant for additional info
	setFlag(ItemSendsGeometryChanges);
#endif
    m_color = Qt::white;
    m_columnList = new ColumnList(this);
    setHub(new BoxSideHub(this));
	connect(m_columnList, SIGNAL(columnInserted(int)), this, SLOT(updateLayout()));
	connect(m_columnList, SIGNAL(columnRemoved(int)), this, SLOT(updateLayout()));
    connect(m_columnList, SIGNAL(columnChanged(int)), this, SLOT(updateLayout()));
}

QRectF DatabaseTable::boundingRect() const
{
	return m_outerRect;
}

QList<Column *> DatabaseTable::primaryKeys() const
{
	QList<Column *> result;
    for(Column *column : m_columnList->columns())
		if (column->isPrimaryKey())
			result.append(column);
	return result;
}

void DatabaseTable::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	QFont font = scene()->font();
	QFont boldFont = font;
	boldFont.setBold(true);
	QFont boldFontWithUnderline = boldFont;
	boldFontWithUnderline.setUnderline(true);

	QPen pen(QPen(QColor(0, 0, 0)));
	pen.setJoinStyle(Qt::MiterJoin);
	pen.setWidth(1);
	QPen borderPen(pen);
	borderPen.setWidthF(1.3);
    if (isSelected()) {
		borderPen.setColor(QColor(0, 96, 255));
    }

    QPen reqPen(QPen(QColor(0, 96, 255)));
    reqPen.setJoinStyle(Qt::MiterJoin);
    reqPen.setWidth(1);

	painter->setPen(pen);
	painter->setFont(font);

	painter->fillRect(m_outerRect, color());

	// Draw the table name
	painter->fillRect(m_nameBgRect, QColor(205, 205, 205));
	painter->drawLine(m_nameBgRect.bottomLeft(), m_nameBgRect.bottomRight());
	painter->drawText(m_namePos, m_name);
	
	painter->drawLine(
		QPointF(m_leftSideWidth, m_nameBgRect.bottom()),
		QPointF(m_leftSideWidth, m_outerRect.bottom()));
	
	// Draw the table name
	QPointF colPos = m_firstColPos;
	QPointF leftSizePos = colPos - QPointF(m_leftSideWidth, 0);
	int i = 0;
    for (Column *column : m_columnList->columns()) {
        bool isBold = column->isRequired();
        painter->setPen( isBold ? reqPen : pen);
		painter->setFont(isBold
			? (column->isPrimaryKey() ? boldFontWithUnderline : boldFont)
			: font);        
		painter->drawText(colPos, column->name());
		if (!m_columnLabels[i].isEmpty()) {
			painter->setFont(isBold ? boldFont : font);
			painter->drawText(leftSizePos, m_columnLabels[i]);
		}
		colPos += m_colPosIncrement;
		leftSizePos += m_colPosIncrement;
		++i;
    }
	
	// Draw the outside border
	painter->setPen(borderPen);
	painter->drawRect(m_outerRect);
}

void DatabaseTable::setName(const QString &name)
{
	m_name = name;
	updateLayout();
	emit propertyChanged("name", name);
}

void DatabaseTable::setColor(const QColor &color)
{
	m_color = color;
	updateLayout();
	emit propertyChanged("color", color);
}

void DatabaseTable::setInitialName(int counter)
{
	setName(counter > 1 ? QString("table_%1").arg(counter) : "table");
}

QVariant DatabaseTable::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (change == ItemSceneHasChanged) {
		updateLayout();
	}
	if (change == ItemPositionChange) {
        auto model = qobject_cast<DiagramDocument *>(scene());
		if (model) {
			emit model->itemMoved(this);
		}
	}
	if (change == ItemPositionHasChanged) {
        auto model = qobject_cast<DiagramDocument *>(scene());
		if (model)
			emit model->itemHasMoved(this);
	}
	return QGraphicsItem::itemChange(change, value);
}

void DatabaseTable::updateLayout()
{
	if (!scene())
		return;

	prepareGeometryChange();
	document()->updatePositions(this);

	QFont font = scene()->font();
	QFontMetricsF fontMetrics(font);

	QFont boldFont = font;
	boldFont.setBold(true);
	QFontMetricsF boldFontMetrics(boldFont);

    #if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
        qreal spaceWidth = fontMetrics.horizontalAdvance(' ');
        qreal nameWidth = fontMetrics.horizontalAdvance(m_name);
        m_leftSideWidth = fontMetrics.horizontalAdvance("PK");
    #else
        qreal spaceWidth = fontMetrics.width(' ');
        qreal nameWidth = fontMetrics.width(m_name);
        m_leftSideWidth = fontMetrics.width("PK");
    #endif
	qreal height = fontMetrics.height();

	qreal width = nameWidth + spaceWidth * 2;

	qreal maxColumnWidth = 0;
    m_columnLabels.clear();
    for (Column *column : m_columnList->columns()) {
        #if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
            qreal columnWidth = column->isRequired() ? boldFontMetrics.horizontalAdvance(column->name()) : fontMetrics.horizontalAdvance(column->name());
        #else
            qreal columnWidth = column->isRequired() ? boldFontMetrics.width(column->name()) : fontMetrics.width(column->name());
        #endif
		maxColumnWidth = qMax(maxColumnWidth, columnWidth);
		qreal columnLeftSideWidth = 0;
		QStringList label;
		if (column->isPrimaryKey()) {
			label << "PK";
		}
		if (column->isForeignKey()) {
			label << "FK";
		}
		QString labelStr = label.join(",");
		m_columnLabels << labelStr;

        #if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
            if (column->isRequired()) {
                columnLeftSideWidth = boldFontMetrics.horizontalAdvance(labelStr);
            }
            else {
                columnLeftSideWidth = fontMetrics.horizontalAdvance(labelStr);
            }
        #else
            if (column->isRequired()) {
                columnLeftSideWidth = boldFontMetrics.width(labelStr);
            }
            else {
                columnLeftSideWidth = fontMetrics.width(labelStr);
            }
        #endif
		m_leftSideWidth = qMax(m_leftSideWidth, columnLeftSideWidth);
	}
	m_leftSideWidth += spaceWidth * 2;
	width = qMax(nameWidth + spaceWidth * 4, m_leftSideWidth + maxColumnWidth + spaceWidth * 2);

	qreal nameHeight = height + spaceWidth;
	m_outerRect = QRectF(0, 0, width, nameHeight + qMax(0.66, qreal(m_columnList->columnCount())) * fontMetrics.lineSpacing()
		- fontMetrics.leading() + fontMetrics.descent());

	m_nameBgRect = QRectF(0, 0, width, nameHeight);
	m_namePos = QPointF((width - nameWidth) / 2, fontMetrics.ascent() + spaceWidth / 2);

	m_firstColPos = QPointF(m_leftSideWidth + spaceWidth * 1 - 1, nameHeight + fontMetrics.ascent() + 2);
    m_colPosIncrement = QPointF(0, fontMetrics.lineSpacing());

	update();
}

void DatabaseTable::setColumns(ColumnList *columnList)
{
    while (m_columnList->columnCount() >0 )
        m_columnList->removeColumn(0);

    for (int i=0; i<columnList->columnCount(); i++)
        m_columnList->appendColumn(columnList->column(i));
}

void DatabaseTable::loadFromXml(QDomElement element, DiagramDocument *document)
{
    Q_UNUSED(document);

    DiagramItem::loadFromXml(element);
    setName(element.attribute("name"));

    QDomElement columnElement = element.firstChildElement("column");
    while (!columnElement.isNull()) {
        Column *column = new Column();
        column->setName(columnElement.attribute("name"));
        column->setDataType(columnElement.attribute("data-type"));
        column->setRequired(columnElement.attribute("required").toInt());
        //column->setPrimaryKey(columnElement.attribute("primary-key").toInt());
        columnList()->appendColumn(column);
        columnElement = columnElement.nextSiblingElement("column");
    }
}

void DatabaseTable::saveToXml(QDomElement element, QDomDocument doc)
{
    DiagramItem::saveToXml(element, doc);
    element.setAttribute("name", name());

    for(Column *column : columnList()->columns()) {
		QDomElement columnElement = doc.createElement("column");
        columnElement.setAttribute("name", column->name());
        columnElement.setAttribute("data-type", column->dataType());
        columnElement.setAttribute("required", column->isRequired());
        //columnElement.setAttribute("primary-key", column->isPrimaryKey());
        element.appendChild(columnElement);
    }
}

void DatabaseTable::getSelectedColumns(QString &value)
{
    int selectedCount = 0;
    QString str;
    for (int i=0; i < columnList()->columns().count(); i++) {
        if (columnList()->column(i)->isRequired()) {
            selectedCount++;
            if (!str.isEmpty())
                str = str + ", ";
            str = str + name()+"."+columnList()->column(i)->name();
        }
    }

    if (columnList()->columns().count() == selectedCount)
        str = name()+".*";

    if (!str.isEmpty()) {
        if (value.isEmpty())
            value = str;
        else
            value = value+", "+str;
    }
}
