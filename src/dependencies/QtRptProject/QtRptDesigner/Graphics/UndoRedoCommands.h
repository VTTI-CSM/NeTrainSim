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

#ifndef UNDOREDOCOMMANDS_H
#define UNDOREDOCOMMANDS_H

#include <QUndoCommand>
#include <QVariant>
#include <QPointer>
#include <QPointF>
#include <QLatin1String>
#include "GraphicsBox.h"
#include "GraphicsScene.h"

class GraphicsBox;
class GraphicsScene;

struct ItemsAndParams {
    QGraphicsItem *item;
    QPointF oldPos;
    QPointF newPos;
    QList<QPointF> oldPointList;
    QList<QPointF> newPointList;
    qreal oldWidth;     //for box
    qreal oldHeight;    //for box
    qreal newWidth;     //for box
    qreal newHeight;    //for box
};

class MoveGItemCommand : public QUndoCommand
{
public:
    MoveGItemCommand(const ItemsAndParams &itm, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    ItemsAndParams m_itm;

};

class MoveLineCommand : public QUndoCommand
{
public:
    MoveLineCommand(const ItemsAndParams &itm, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    ItemsAndParams m_itm;

};

//************************************************************************************************

class DelItemCommand : public QUndoCommand
{
public:
    DelItemCommand(GraphicsScene *scene, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    QList<QGraphicsItem *> itemList;
    QList<QGraphicsItem *> parentList;
    GraphicsScene *myGraphicsScene;
    //QWidget *pw;
    //MainWindow *m_mainWindow;
    //bool m_create;
    //QList<PairItem> m_dataList;

};

//************************************************************************************************

class AddCommand : public QUndoCommand
{
public:
    AddCommand(QGraphicsItem *item, GraphicsScene *graphicsScene, QGraphicsItem *p_item,
               QUndoCommand *parent = 0);
    ~AddCommand();

    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

private:
    QGraphicsItem *myDiagramItem;
    QGraphicsItem *mpItem;
    GraphicsScene *myGraphicsScene;
    QPointF initialPosition;
};

//************************************************************************************************

struct PairCont {
    QByteArray oldBArray;
    QByteArray newBArray;
    GraphicsHelperClass *gHelper;
};

typedef QList<QGraphicsItem *> QGraphicsItemList;
typedef QList<GraphicsHelperClass *> GraphicsHelperList;
typedef QList<QPair<QByteArray, GraphicsHelperClass*> > BArrayList;

class ParamCommand : public QUndoCommand
{
public:
    ParamCommand(QList<PairCont>& list, GraphicsScene *scene, QUndoCommand *parent = 0);
    ~ParamCommand();

    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

    static BArrayList getBArrayFromContList(GraphicsHelperList contList);
    static QList<PairCont> compoundArrays(BArrayList oldList, BArrayList newList);

private:
    GraphicsScene *myGraphicsScene;
    QList<PairCont> m_dataList;

};

#endif // UNDOREDOCOMMANDS_H
