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

#ifndef DATABASEMODEL_H
#define DATABASEMODEL_H

#include <QUndoStack>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QDomDocument>
#include <QGraphicsLineItem>
#include <QSet>
#include <QUuid>

#include "columnlist.h"

class DatabaseTable;
class DatabaseRelationship;
class Line;
class DiagramItem;
class DiagramObject;

class QPainter;
class QPrinter;

class DiagramDocument : public QGraphicsScene
{
	Q_OBJECT
    /*
	Q_PROPERTY(int gridSize READ gridSize WRITE setGridSize)
    Q_PROPERTY(bool gridVisible READ isGridVisible WRITE setGridVisible)
	Q_PROPERTY(QColor gridColor READ gridColor WRITE setGridColor)*/

public:
    DiagramDocument(QWidget *parentWidget = 0);
	
	enum Mode {
		Select = 0,
		AddTable,
		AddRelation
	};

    Mode mode();
    void setMode(Mode mode);

    DatabaseTable *selectedTable();
    void deleteSelectedItems();

    QList<DiagramItem *> selectedItems();

    void save(QSharedPointer<QDomDocument> xmlDoc, QDomElement element);
    bool load(QDomElement element);

    //QString fileName() { return m_fileName; }
    //void setFileName(const QString &fileName) { m_fileName = fileName; }

    DiagramItem *itemById(const QUuid &id);
    QUndoStack *undoStack() const { return m_undoStack; }

    void itemMoved(DiagramItem *table);
    void itemHasMoved(DiagramItem *table);

    QList< ::Line *> findConnections(DiagramObject *object);
    template <class T>QList<T *> itemsByType();

    void addItemLater(DiagramItem *item);
    void removeItemLater(DiagramItem *item);
    void passColumnsForDroping(ColumnList *columnList);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void drawBackground(QPainter *painter, const QRectF &rect);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

private:
    class DiagramDocumentPrivate;
    DiagramDocumentPrivate *const d;

    void updateLines(QSet<DiagramObject*> objects);

    bool m_trackingMoves;
    QMap<DiagramItem *, QPointF> m_movedItems;

    QUndoStack *m_undoStack;
    Mode m_mode;
    //QString m_fileName;
    QGraphicsLineItem *m_line;
    ColumnList *m_columnList;
    QWidget *m_parentWidget;
    void makeSQLQuery();

signals:
    void modeChanged(DiagramDocument::Mode mode);
    void sqlChanged(const QString value);

    friend class DatabaseTable;

public slots:
    void updatePositions(DiagramObject *object, bool force = false);
    void updateLineLayout(::Line *line);

private slots:
    void _updateLines();


};

#endif
