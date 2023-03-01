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

#include "UndoRedoCommands.h"
#include "RepScrollArea.h"

MoveGItemCommand::MoveGItemCommand(const ItemsAndParams &itm, QUndoCommand *parent)
: QUndoCommand(parent), m_itm(itm)
{
    setText(QObject::tr("Moving box"));
}

void MoveGItemCommand::redo()
{
    auto box = qgraphicsitem_cast<GraphicsBox*>(m_itm.item);
    box->setPos(m_itm.newPos);
    box->setWidth(m_itm.newWidth);
    box->setHeight(m_itm.newHeight);
}

void MoveGItemCommand::undo()
{
    auto box = qgraphicsitem_cast<GraphicsBox*>(m_itm.item);
    box->setPos(m_itm.oldPos);
    box->setWidth(m_itm.oldWidth);
    box->setHeight(m_itm.oldHeight);
    box->setSelected(true);
}


MoveLineCommand::MoveLineCommand(const ItemsAndParams &itm, QUndoCommand *parent)
: QUndoCommand(parent), m_itm(itm)
{
    setText(QObject::tr("Moving line"));
}

void MoveLineCommand::redo()
{
    auto line = static_cast<GraphicsLine*>(m_itm.item);
    line->setPos(m_itm.newPos);
    line->setPointList(m_itm.newPointList);
}

void MoveLineCommand::undo()
{
    auto line = static_cast<GraphicsLine*>(m_itm.item);
    line->setPos(m_itm.oldPos);
    line->setPointList(m_itm.oldPointList);
}

//************************************************************************************************

DelItemCommand::DelItemCommand(GraphicsScene *scene, QUndoCommand *parent)
: QUndoCommand(parent)
{
    myGraphicsScene = scene;

    for (auto &item : scene->items()) {
        bool isSelected = false;
        GraphicsBox* box = nullptr;
        if (item->type() == ItemType::GBand) {
            box = static_cast<GraphicsBox*>(item);
            isSelected = box->isSelected();
        }
        if (item->type() == ItemType::GBox) {
            box = static_cast<GraphicsBox*>(item);
            isSelected = box->isSelected();
        }
        GraphicsLine* line = nullptr;
        if (item->type() == ItemType::GLine) {
            line = static_cast<GraphicsLine*>(item);
            isSelected = line->isSelected();
        }
        if (isSelected)
            itemList << item;
    }

    setText(QObject::tr("Delete"));
}

void DelItemCommand::undo()
{
    auto area = qobject_cast<RepScrollArea*>(myGraphicsScene->parent());

    quint32 i = 0;
    for (auto &item : itemList) {
        myGraphicsScene->addItem(item);
        if (item->type() == ItemType::GBox || item->type() == ItemType::GLine) {
            item->setParentItem(parentList[i]);
            area->newFieldTreeItem(item);
        }
        if (itemList[i]->type() == ItemType::GBand) {
            area->newFieldTreeItem(item);
        }
        i++;
    }
    myGraphicsScene->update();
}

void DelItemCommand::redo()
{
    for (auto &cont1 : itemList) {
        parentList << cont1->parentItem();
        myGraphicsScene->removeItem(cont1);
    }
}

//************************************************************************************************

AddCommand::AddCommand(QGraphicsItem *item, GraphicsScene *scene, QGraphicsItem *p_item, QUndoCommand *parent)
: QUndoCommand(parent)
{
    static int itemCount = 0;

    myGraphicsScene = scene;
    mpItem = p_item;
    myDiagramItem = item;
    initialPosition = item->scenePos();
    scene->update();
    ++itemCount;
    setText(QObject::tr("Add"));
}

AddCommand::~AddCommand()
{
    if (!myDiagramItem)
        delete myDiagramItem;
}

void AddCommand::undo()
{
    myGraphicsScene->removeItem(myDiagramItem);
    myGraphicsScene->update();
}

void AddCommand::redo()
{
    if (myDiagramItem->scene() == nullptr)
        myGraphicsScene->addItem(myDiagramItem);
    if (mpItem != nullptr) {
		myDiagramItem->setParentItem(mpItem);

        auto area = qobject_cast<RepScrollArea*>(myGraphicsScene->parent());
        area->newFieldTreeItem(myDiagramItem);
	}
    myDiagramItem->setPos(initialPosition);
    myGraphicsScene->clearSelection();
    myGraphicsScene->update();
}

//************************************************************************************************

ParamCommand::ParamCommand(QList<PairCont>& list, GraphicsScene *scene, QUndoCommand *parent)
: QUndoCommand(parent)
{
    myGraphicsScene = scene;
    m_dataList = list;
    this->setText(QObject::tr("Changing Container's parameters"));
}

ParamCommand::~ParamCommand()
{}

void ParamCommand::undo()
{
    for (auto &pair : m_dataList) {
        QDataStream in(pair.oldBArray);
        auto second = qobject_cast<GraphicsHelperClass *>(pair.gHelper);
        if (second == nullptr)
            continue;
        in >> *second;
        pair.gHelper = second;
    }
    myGraphicsScene->update();
}

void ParamCommand::redo()
{
    for (auto &pair : m_dataList) {
        QDataStream in(pair.newBArray);
        auto second = qobject_cast<GraphicsHelperClass *>(pair.gHelper);
        in >> *second;
        pair.gHelper = second;
    }
    myGraphicsScene->update();
}

BArrayList ParamCommand::getBArrayFromContList(GraphicsHelperList contList)
{
    BArrayList list;
    for (auto &cont1 : contList) {
        QByteArray byteArray;
        QDataStream out(&byteArray, QIODevice::WriteOnly);
        out << *cont1;

        QPair<QByteArray, GraphicsHelperClass*> pair;
        pair.first = byteArray;
        pair.second = cont1;
        list.append(pair);
    }
    return list;
}

QList<PairCont> ParamCommand::compoundArrays(BArrayList oldList, BArrayList newList)
{
    QList<PairCont> list;
    for (auto &oldPair : oldList) {
        for (auto &newPair : newList) {
            if (oldPair.second == newPair.second) {
                PairCont pair;
                pair.oldBArray = oldPair.first;
                pair.gHelper   = oldPair.second;
                pair.newBArray = newPair.first;
                list.append(pair);
            }
        }
    }
    return list;
}

