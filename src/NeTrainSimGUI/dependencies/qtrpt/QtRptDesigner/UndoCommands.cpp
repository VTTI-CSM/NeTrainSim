/*
Name: QtRpt
Version: 1.5.5
Web-site: http://www.qtrpt.tk
Programmer: Aleksey Osipov
E-mail: aliks-os@ukr.net
Web-site: http://www.aliks-os.tk

Copyright 2012-2015 Aleksey Osipov

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

#include <QDebug>
#include "UndoCommands.h"

GeometryContainerCommand::GeometryContainerCommand(RptContainerList contList, QUndoCommand *parent) : QUndoCommand(parent) {
    m_contList = contList;

    foreach (RptContainer *cont, contList) {
        TContainerLine *contLine = qobject_cast<TContainerLine *>(cont);
        if (contLine != 0) {
            m_oldP1.append(contLine->m_oldP1);
            m_oldP2.append(contLine->m_oldP2);

            m_newP1.append(contLine->cs->geometry());
            m_newP2.append(contLine->ce->geometry());

        } else {
            m_oldP1.append(cont->getOldGeom());
            m_newP1.append(cont->geometry());
        }
    }
    this->setText(QObject::tr("Changing Container's geometry"));
}

void GeometryContainerCommand::redo() {
    for (int i=0; i<m_contList.size(); i++) {
        TContainerLine *contLine = qobject_cast<TContainerLine *>(m_contList[i]);
        if (contLine != 0) {
            contLine->movePoint(contLine->cs, m_newP1.at(i));
            contLine->movePoint(contLine->ce, m_newP2.at(i));
            contLine->move(-10,-10);
        } else {
            m_contList[i]->setGeometry(m_newP1.at(i));
        }
    }
}

void GeometryContainerCommand::undo() {
    for (int i=0; i<m_contList.size(); i++) {
        TContainerLine *contLine = qobject_cast<TContainerLine *>(m_contList[i]);
        if (contLine != 0) {
            contLine->movePoint(contLine->cs, m_oldP1.at(i));
            contLine->movePoint(contLine->ce, m_oldP2.at(i));
            contLine->move(-10,-10);
        } else {
            m_contList[i]->setGeometry(m_oldP1.at(i));
        }
    }
}

//************************************************************************************************

AddContainerCommand::AddContainerCommand(RptContainerList contList, QUndoCommand *parent) : QUndoCommand(parent) {
    m_mainWindow = getMW();

    foreach (RptContainer *cont1, contList) {
        cont1->setProperties();
        pw = cont1->parentWidget();

        PairCont pair;
        QByteArray byteArray;
        QDataStream out(&byteArray, QIODevice::WriteOnly);

        TContainerField *contF = qobject_cast<TContainerField *>(cont1);
        if (contF != 0) {
            out << *contF;
            pair.second = contF;
        }
        TContainerLine *contL = qobject_cast<TContainerLine *>(cont1);
        if (contL != 0) {
            out << *contL;
            pair.second = contL;
        }

        pair.first = byteArray;
        pair.third = cont1->getType();
        m_dataList << pair;
    }
    m_create = true;
    this->setText(QObject::tr("Adding Container"));
}

void AddContainerCommand::redo() {
    if (m_create) {
        m_create = false;
        return;
    }
    for (int i=0; i<m_dataList.size(); i++) {
        PairCont pair = m_dataList[i];
        QDataStream in(pair.first);

        if (pair.third != QtRptName::Line) {
            TContainerField *second = new(pair.second) TContainerField(pw,QPoint(0,0));
            in >> *second;
            pair.second = second;
        } else {
            TContainerLine *second = new(pair.second) TContainerLine(pw,QPoint(0,0));
            in >> *second;
            pair.second = second;
        }

        m_mainWindow->setContainerConnections(pair.second);
        ReportBand *band = qobject_cast<ReportBand *>(pw->parentWidget());
        band->newFieldTreeItem(pair.second);
        m_dataList[i] = pair;
    }
}

void AddContainerCommand::undo() {
    foreach (PairCont pair, m_dataList) {
        (*pair.second).RptContainer::~RptContainer();
    }
    m_mainWindow->widgetInFocus = 0;//pw->parentWidget();
}

//************************************************************************************************

DelContainerCommand::DelContainerCommand(RptContainerList contList, QUndoCommand *parent) : QUndoCommand(parent) {
    m_mainWindow = getMW();

    foreach (RptContainer *cont1, contList) {
        cont1->setProperties();
        pw = cont1->parentWidget();

        PairCont pair;
        QByteArray byteArray;
        QDataStream out(&byteArray, QIODevice::WriteOnly);

        TContainerField *contF = qobject_cast<TContainerField *>(cont1);
        if (contF != 0) {
            out << *contF;
            pair.second = contF;
        }
        TContainerLine *contL = qobject_cast<TContainerLine *>(cont1);
        if (contL != 0) {
            out << *contL;
            pair.second = contL;
        }

        pair.first = byteArray;
        pair.third = cont1->getType();
        m_dataList << pair;
    }
    m_create = true;
    this->setText(QObject::tr("Deleting Container"));
}

void DelContainerCommand::redo() {
    if (m_create) {
        m_create = false;
        return;
    }
    foreach (PairCont pair, m_dataList) {
        (*pair.second).RptContainer::~RptContainer();
    }
}

void DelContainerCommand::undo() {
    for (int i=0; i<m_dataList.size(); i++) {
        PairCont pair = m_dataList[i];
        QDataStream in(pair.first);

        if (pair.third != QtRptName::Line) {
            TContainerField *second = new(pair.second) TContainerField(pw,QPoint(0,0));
            in >> *second;
            pair.second = second;
        } else {
            TContainerLine *second = new(pair.second) TContainerLine(pw,QPoint(0,0));
            in >> *second;
            pair.second = second;
        }

        m_mainWindow->setContainerConnections(pair.second);
        ReportBand *band = qobject_cast<ReportBand *>(pw->parentWidget());
        band->newFieldTreeItem(pair.second);
        m_dataList[i] = pair;
        //emit pair.second->inFocus(true);
    }
}

//************************************************************************************************


ParamsContainerCommand::ParamsContainerCommand(QList<PairCont> list, QUndoCommand *parent) : QUndoCommand(parent) {
    m_dataList = list;
    m_create = true;
    this->setText(QObject::tr("Changing Container's parameters"));
}

void ParamsContainerCommand::redo() {
    if (m_create) {
        m_create = false;
        return;
    }
    for (int i=0; i<m_dataList.size(); i++) {
        PairCont pair = m_dataList[i];
        QDataStream in(pair.fifth);

        if (pair.third != QtRptName::Line) {
            TContainerField *second = qobject_cast<TContainerField *>(pair.second);
            in >> *second;
            pair.second = second;
        } else {
            TContainerLine *second = qobject_cast<TContainerLine *>(pair.second);
            in >> *second;
            pair.second = second;
        }
        pair.second->itemInTree->setText(0,pair.second->objectName());
    }
}

void ParamsContainerCommand::undo() {
    for (int i=0; i<m_dataList.size(); i++) {
        PairCont pair = m_dataList[i];
        QDataStream in(pair.first);
        if (pair.third != QtRptName::Line) {
            TContainerField *second = qobject_cast<TContainerField *>(pair.second);
            if (second == 0) continue;
            in >> *second;
            pair.second = second;
        } else {
            TContainerLine *second = qobject_cast<TContainerLine *>(pair.second);
            if (second == 0) continue;
            in >> *second;
            pair.second = second;
        }
        pair.second->itemInTree->setText(0,pair.second->objectName());
    }
}


BArrayList ParamsContainerCommand::getBArrayFromContList(RptContainerList contList) {
    BArrayList list;
    foreach (RptContainer *cont1, contList) {
        cont1->setProperties();
        QByteArray byteArray;
        QDataStream out(&byteArray, QIODevice::WriteOnly);

        TContainerField *contF = qobject_cast<TContainerField *>(cont1);
        if (contF != 0)
            out << *contF;
        TContainerLine *contL = qobject_cast<TContainerLine *>(cont1);
        if (contL != 0)
            out << *contL;

        QPair<QByteArray, RptContainer*> pair;
        pair.first = byteArray;
        pair.second = cont1;
        list.append(pair);
    }
    return list;
}

QList<PairCont> ParamsContainerCommand::compoundArrays(BArrayList oldList, BArrayList newList) {
    QList<PairCont> list;
    for (int i=0; i<oldList.size(); i++) {
        for (int j=0; j<newList.size(); j++) {
            if (oldList[i].second == newList[j].second) {
                PairCont pair;
                pair.first = oldList[i].first;
                pair.second = oldList[i].second;
                pair.third = oldList[i].second->getType();
                pair.fifth = newList[j].first;
                list.append(pair);
            }
        }
    }
    return list;
}

//************************************************************************************************
