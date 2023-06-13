/*
Name: QtRpt
Version: 3.0.0
Web-site: https://qtrpt.sourceforge.io
Programmer: Aleksey Osipov
E-mail: aliks-os@ukr.net
Web-site: http://www.aliks-os.tk

Copyright 2012-2022 Aleksey Osipov

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <QUndoStack>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLineItem>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QObject>
#include <QList>

#include "GraphicsLine.h"
#include "GraphicsBox.h"
#include "ReportBand.h"
#include "UndoRedoCommands.h"

struct ItemsAndParams;

class GraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    enum Mode {NoMode, SelectObject, DrawLine, DrawContainer};
    GraphicsScene(QObject* parent = nullptr);
    void setMode(Mode mode);
    void addItem(QGraphicsItem * item);
    QUndoStack *undoStack() const { return m_undoStack; }
    void itemMoving(QGraphicsItem *item);
    void itemResizing(QGraphicsItem *item);
    QUndoStack *m_undoStack;
    void unselectAll();
    //void setBackground(QString path);
    bool isShowGrid;
    void setGridStep(double gridStep) {m_gridStep = gridStep;}
    void setMesKoef(double koef) {m_koef = koef;}
    void setMargins(double left, double right, double top, double bottom) {
        m_leftM = left;
        m_rightM = right;
        m_topM = top;
        m_bottomM = bottom;
    }
    bool newLineArrowStart;
    bool newLineArrowEnd;
    void newFieldType(QtRptName::FieldType value) {m_newFieldType = value;}
    void newFieldMenu(QMenu *menu) {m_newFieldMenu = menu;}
    void removeItem(QGraphicsItem *item);
    void itemSelect(QGraphicsItem *item);
    QList<QGraphicsItem*> itemsSelected() {return m_selectedItems;}
    QList<ItemsAndParams> itemsMoved() {return m_movedItems;}

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void drawBackground(QPainter *painter, const QRectF &rect);

private:
    double m_gridStep;
    double m_koef;
    double m_leftM;
    double m_rightM;
    double m_topM;
    double m_bottomM;
    Mode sceneMode;
    QtRptName::FieldType m_newFieldType;
    QMenu *m_newFieldMenu;
    QList<QGraphicsItem*> m_selectedItems;
    bool m_trackingMoves;
    QList<ItemsAndParams> m_movedItems;

public slots:
    void itemRemoving();

signals:
    void mousePos(QPointF pos);
    void sceneModeChanged(QGraphicsItem *item, GraphicsScene::Mode mode);
    void itemSelected(QGraphicsItem *item);
    void itemAdded(QGraphicsItem *item);
    void itemResized(QGraphicsItem *item);
    void sceneClick();
};
