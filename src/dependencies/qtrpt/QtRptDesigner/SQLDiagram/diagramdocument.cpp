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


#include <cmath>
#include "hub.h"
#include "diagramdocument.h"
#include "diagramobject.h"
#include "line.h"
#include "databasetable.h"
#include "column.h"
#include "commands.h"
#include <QGraphicsItem>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QStyle>
#include <QSet>
#include <QPainter>
#include <QPrinter>
#include <QMimeData>
#include "ItemPropertyDlg.h"

class DiagramDocument::DiagramDocumentPrivate
{
public:
    DiagramDocumentPrivate() : updateTimerIsRunning(false)
        {}

	bool updateTimerIsRunning;
	QTimer *updateTimer;
    QList<DiagramItem *> itemsToShow;
	QList<DiagramItem *> itemsToRemove;
    QSet<Line *> linesToUpdate;
    QSet<DiagramObject *> objectsToUpdate;
    QMap<QString, int> counters;
};

static const int UPDATE_DELAY = 2;

DiagramDocument::DiagramDocument(QWidget *parentWidget)
: QGraphicsScene(parentWidget), d(new DiagramDocumentPrivate), m_mode(DiagramDocument::Select), m_line(nullptr)
{
    m_parentWidget = parentWidget;
    m_undoStack = new QUndoStack(this);
    d->updateTimer = new QTimer();
    d->updateTimer->setSingleShot(true);
    d->updateTimer->setInterval(UPDATE_DELAY);
    connect(d->updateTimer, SIGNAL(timeout()), this, SLOT(_updateLines()));
}

void DiagramDocument::updatePositions(DiagramObject *object, bool force)
{
    d->objectsToUpdate.insert(object);
    if (force) {
		d->updateTimer->start(0);
		d->updateTimer->setInterval(UPDATE_DELAY);
	}
	else {
		d->updateTimer->start();
    }
}

void DiagramDocument::_updateLines()
{
    d->updateTimerIsRunning = true;
    QSet<DiagramObject *> objectsToUpdate(d->objectsToUpdate);
    d->objectsToUpdate.clear();
    updateLines(objectsToUpdate);
    d->updateTimerIsRunning = false;
}

void DiagramDocument::updateLines(QSet<DiagramObject *> objectsToUpdate)
{
    QSet<Hub *> hubsToUpdate;
    for(DiagramObject *obj : objectsToUpdate) {
		hubsToUpdate.insert(obj->hub());
        for(Connector *conn : obj->hub()->connectors()) {
			hubsToUpdate.insert(conn->otherEnd()->hub());
		}
	}
    for(Hub *hub : hubsToUpdate) {
        hub->update();
	}
    for(Line *line : d->linesToUpdate) {
		line->updateLayout();
		line->update();
	}
    d->linesToUpdate.clear();

    for(DiagramItem *item : d->itemsToShow) {
		item->show();
	}
    d->itemsToShow.clear();

    for(DiagramItem *item : d->itemsToRemove) {
		removeItem(item);
	}
    d->itemsToRemove.clear();
}

void DiagramDocument::addItemLater(DiagramItem *item)
{
	Q_ASSERT(d->updateTimerIsRunning == false);
	item->hide();
	addItem(item);
	d->itemsToShow.append(item);
	d->updateTimer->start(0);
}

void DiagramDocument::removeItemLater(DiagramItem *item)
{
	Q_ASSERT(d->updateTimerIsRunning == false);
	item->hide();
	d->itemsToRemove.append(item);
	d->updateTimer->start(0);
}

void DiagramDocument::deleteSelectedItems()
{
    for(DiagramItem *item : selectedItems()) {
		DiagramObject *obj = qobject_cast<DiagramObject *>(item);
		if (obj) {
			undoStack()->push(new RemoveObjectCommand(this, obj));
            makeSQLQuery();
        } else {
			Line *line = qobject_cast<Line *>(item);
			if (line) {
				undoStack()->push(new RemoveLineCommand(this, line));
                makeSQLQuery();
			}
		}
    }
}

void DiagramDocument::save(QSharedPointer<QDomDocument> xmlDoc, QDomElement element)
{
    QDomElement root = xmlDoc->createElement("diagram");
    element.appendChild(root);

    for (DiagramObject *item : itemsByType<DiagramObject>()) {
        QDomElement element = xmlDoc->createElement("item");
        root.appendChild(element);
        item->saveToXml(element, *xmlDoc);
	}

    for (Line *item : itemsByType<Line>()) {
        QDomElement element = xmlDoc->createElement("item");
        root.appendChild(element);
        item->saveToXml(element, *xmlDoc);
	}
}

bool DiagramDocument::load(QDomElement element)
{
    QDomElement root = element.firstChildElement("diagram");
    QDomElement itemElement = root.firstChildElement("item");
	while (!itemElement.isNull()) {
		QString itemTypeName = itemElement.attribute("type");
        DiagramItem *item;
        if (itemTypeName == "database-table") item = new DatabaseTable;
        if (itemTypeName == "database-relationship") item = new DatabaseRelationship;
        if (item == nullptr) {
			qWarning() << "Unknown item type:" << itemTypeName;
        } else {
			item->loadFromXml(itemElement, this);
			addItem(item);
		}
		itemElement = itemElement.nextSiblingElement("item");
    }
	return true;
}

template <class T> QList<T *>
DiagramDocument::itemsByType()
{
    QList<T *> result;
    for (QGraphicsItem *item : items()) {
        T *typedItem = dynamic_cast<T *>(item);
        if (typedItem) {
            result.append(typedItem);
        }
        //if (item->type() == T::Type) {
        //	result.append(static_cast<T *>(item));
        //}
    }
    return result;
}

DatabaseTable *DiagramDocument::selectedTable()
{
    QList<DiagramItem *> items = selectedItems();
    if (items.size() != 1)
        return nullptr;
    return qgraphicsitem_cast<DatabaseTable *>(items[0]);
}

QList<Line *> DiagramDocument::findConnections(DiagramObject *object)
{
    QList<Line *> result;
    for(Line *connection : itemsByType<Line>())
        if (connection->connector(0)->connectedObject() == object || connection->connector(1)->connectedObject() == object)
            result.append(connection);
    return result;
}

void DiagramDocument::itemMoved(DiagramItem *item)
{
    if (m_trackingMoves) {
        if (!m_movedItems.contains(item)) {
            m_movedItems[item] = item->pos();
        }
    }
}

void DiagramDocument::itemHasMoved(DiagramItem *item)
{
    DiagramObject *obj = static_cast<DiagramObject *>(item);
    if (obj) {
        updatePositions(obj);
    }
}

void DiagramDocument::updateLineLayout(Line *line)
{
    d->linesToUpdate.insert(line);
}

DiagramItem *DiagramDocument::itemById(const QUuid &id)
{
    for(DiagramItem *item : itemsByType<DiagramItem>())
		if (item->id() == id)
			return item;
	return 0;
}

QList<DiagramItem *> DiagramDocument::selectedItems()
{
    QList<DiagramItem *> result;
    for(QGraphicsItem *item : QGraphicsScene::selectedItems()) {
        DiagramItem *typedItem = dynamic_cast<DiagramItem *>(item);
        if (typedItem) {
            result.append(typedItem);
        }
    }
    return result;
}

DiagramDocument::Mode DiagramDocument::mode()
{
    return m_mode;
}

void DiagramDocument::setMode(Mode mode)
{
    m_mode = mode;
    emit modeChanged(mode);
}

void DiagramDocument::drawBackground(QPainter *painter, const QRectF &rect)
{
    /*Drawing grid*/
    const int pointBufferSize = 5000;
    static QPoint pointBuffer[pointBufferSize];
    int gridSize = 10;
    int x0 = gridSize * floor(rect.left() / gridSize);
    int y0 = gridSize * floor(rect.top() / gridSize);
    int x1 = gridSize * ceil(rect.right() / gridSize);
    int y1 = gridSize * ceil(rect.bottom() / gridSize);
    painter->save();
    QPen pen;
    pen.setColor(QColor(185, 185, 185));
    pen.setWidth(0);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing, false);
    int pointsUsed = 0;
    for (int x = x0; x < x1; x += gridSize) {
        for (int y = y0; y < y1; y += gridSize) {
            pointBuffer[pointsUsed].setX(x);
            pointBuffer[pointsUsed].setY(y);
            pointsUsed++;
            if (pointsUsed == pointBufferSize) {
                painter->drawPoints(pointBuffer, pointsUsed);
                pointsUsed = 0;
            }
        }
    }
    if (pointsUsed > 0) {
        painter->drawPoints(pointBuffer, pointsUsed);
        pointsUsed = 0;
    }
    painter->restore();
}

void DiagramDocument::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (m_line) {
        m_line->setLine(QLineF(m_line->line().p1(), event->scenePos()));
        event->accept();
        return;
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void DiagramDocument::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    setMode(Select);
    QGraphicsScene::mousePressEvent(event);
    if (m_mode == Select) {
        QList<DiagramItem *> items = selectedItems();
        if (items.isEmpty()) return;
        DiagramItem *item = items.first();        

        ItemPropertyDlg *dlg = new ItemPropertyDlg(this->m_parentWidget);
        dlg->showThis(item);
        makeSQLQuery();
    }
}

void DiagramDocument::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_movedItems.clear();
    m_trackingMoves = true;
    if (m_mode == AddRelation && event->button() == Qt::LeftButton) {
        m_line = new QGraphicsLineItem();
        m_line->setLine(QLineF(event->scenePos(), event->scenePos()));
        QPen pen;
        pen.setStyle(Qt::CustomDashLine);
        pen.setDashOffset(7);
        pen.setDashPattern(QVector<qreal>() << 3 << 3);
        m_line->setPen(pen);
        m_line->setZValue(1000.0);
        addItem(m_line);
        event->accept();
        return;
    }
    QGraphicsScene::mousePressEvent(event);
}

void DiagramDocument::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_line) {
        removeItem(m_line);
        DatabaseTable *source = qgraphicsitem_cast<DatabaseTable *>(itemAt(m_line->line().p1(), QTransform()));
        DatabaseTable *target = qgraphicsitem_cast<DatabaseTable *>(itemAt(m_line->line().p2(), QTransform()));
        if (source && target && source != target) {
            Line *line = new DatabaseRelationship();
            line->connector(0)->setHub(source->hub());
            line->connector(1)->setHub(target->hub());
            undoStack()->push(new AddLineCommand(this, line));

            ItemPropertyDlg *dlg = new ItemPropertyDlg(this->m_parentWidget);
            dlg->showThis(line);
            if (dlg->result() == QDialog::Rejected) {
                undoStack()->undo();
            } else {
                makeSQLQuery();

                QMapIterator<DiagramItem *, QPointF> i(m_movedItems);
                while (i.hasNext()) {
                    i.next();
                    undoStack()->push(new MoveItemCommand(i.key(), i.value(), i.key()->pos()));
                }
            }
        }
        delete m_line;
        m_line = nullptr;
        setMode(Select);
        event->accept();
        return;
    }

    QGraphicsScene::mouseReleaseEvent(event);
    m_trackingMoves = false;
}

void DiagramDocument::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->accept();
}

void DiagramDocument::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    event->accept();
}

void DiagramDocument::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    QByteArray encoded = mimeData->data("application/x-qabstractitemmodeldatalist");
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    while (!stream.atEnd()) {
        int row, col;
        QMap<int,  QVariant> roleDataMap;
        stream >> row >> col >> roleDataMap;
        QString dropped = roleDataMap[0].toString();

        setMode(DiagramDocument::AddTable);
        DatabaseTable *table = new DatabaseTable();
        table->createId();
        table->setInitialName(1 + d->counters[table->typeName()]++);
        table->setName(dropped);
        table->setPos(event->scenePos());
        undoStack()->push(new AddObjectCommand(this, table));
        clearSelection();
        table->setSelected(true);
        setMode(Select);
        if (m_columnList != 0) {
            table->setColumns(m_columnList);
            makeSQLQuery();
        }
    }
    event->accept();
}

void DiagramDocument::makeSQLQuery()
{
    QList<DatabaseTable *> tmpLst;
    QString sqlQueryStr;
    QString selectedColumns;
    for(DatabaseTable *item : itemsByType<DatabaseTable>()) {
        if (item->hub()) {
            if (item->hub()->outgoingLines().count() == 0 && item->hub()->incomingLines().count() == 0 ) { //have outgoing
                if (sqlQueryStr.isEmpty())
                    sqlQueryStr = "SELECT * FROM "+item->name();
                else
                    sqlQueryStr = sqlQueryStr + ",\n"+item->name();

                if (tmpLst.indexOf(item) == -1)
                    item->getSelectedColumns(selectedColumns);
                tmpLst.append(item);
            }
        }        
    }

    QList<DatabaseRelationship *> relList = itemsByType<DatabaseRelationship>();
    for (int itn=0; itn < relList.count(); itn++) {
        DatabaseRelationship *relation = relList.at(itn);
        DatabaseTable *childTable = relation->childTable();
        DatabaseTable *parentTable = relation->parentTable();

        if (sqlQueryStr.isEmpty()) {
            sqlQueryStr = "SELECT * FROM "+childTable->name();
        } else {
            if (itn == 0)
                sqlQueryStr = sqlQueryStr + ",\n"+childTable->name();  //!!!!!!!!!!!!
        }

        if (tmpLst.indexOf(childTable) == -1)
            childTable->getSelectedColumns(selectedColumns);
        if (tmpLst.indexOf(parentTable) == -1)
            parentTable->getSelectedColumns(selectedColumns);

        tmpLst.append(childTable);
        tmpLst.append(parentTable);

        QString childColumn;
        QString parentColumn;
        for (int i=0; i<childTable->columnList()->columnCount(); i++) {
            if (childTable->columnList()->column(i) == relation->childColumn() ) {
                childColumn = childTable->columnList()->column(i)->name() ;
            }
        }
        for (int i=0; i<parentTable->columnList()->columnCount(); i++) {
            if (parentTable->columnList()->column(i) == relation->parentColumn() ) {
                parentColumn = parentTable->columnList()->column(i)->name() ;
            }
        }
        bool diffPair = false;
        if (itn > 0) {
            DatabaseRelationship *relationPrev = relList.at(itn-1);
            DatabaseTable *childTablePrev = relationPrev->childTable();
            DatabaseTable *parentTablePrev = relationPrev->parentTable();

            if ( (childTable != childTablePrev && childTable != parentTablePrev) && (parentTable != childTablePrev && parentTable != parentTablePrev) )
                diffPair = true;
        }
        if (diffPair) {
            sqlQueryStr = sqlQueryStr + ",\n"+childTable->name();
            sqlQueryStr = sqlQueryStr + "\nINNER JOIN "+parentTable->name()+" ON "+parentTable->name()+"."+parentColumn+" = "+childTable->name()+"."+childColumn;
        } else
            sqlQueryStr = sqlQueryStr + "\nINNER JOIN "+parentTable->name()+" ON "+parentTable->name()+"."+parentColumn+" = "+childTable->name()+"."+childColumn;
    }

    sqlQueryStr.replace("FROM","\nFROM");
    sqlQueryStr.replace("*",selectedColumns);

    emit sqlChanged(sqlQueryStr);
}

void DiagramDocument::passColumnsForDroping(ColumnList *columnList)
{
    m_columnList = columnList;
}
