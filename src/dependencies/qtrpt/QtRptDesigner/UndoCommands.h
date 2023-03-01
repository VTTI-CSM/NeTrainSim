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

#ifndef UNDOCOMMANDS_H
#define UNDOCOMMANDS_H

#include <QUndoCommand>
#include "RptContainer.h"
#include "TContainerField.h"
#include "ReportBand.h"
#include "mainwindow.h"

struct PairCont {
    QByteArray first;
    RptContainer *second;
    FieldType third;
    QByteArray fifth;
};


class GeometryContainerCommand : public QUndoCommand
{
public:
    GeometryContainerCommand(const RptContainerList contList, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    RptContainerList m_contList;
    QList<QRect> m_oldP1, m_oldP2, m_newP1, m_newP2;
};

//************************************************************************************************

class MainWindow;
class AddContainerCommand : public QUndoCommand
{
public:
    AddContainerCommand(RptContainerList contList, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:   
    QWidget *pw;
    MainWindow *m_mainWindow;
    bool m_create;
    QList<PairCont> m_dataList;

};

//************************************************************************************************

class DelContainerCommand : public QUndoCommand
{
public:
    DelContainerCommand(RptContainerList contList, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    QWidget *pw;
    MainWindow *m_mainWindow;
    bool m_create;
    QList<PairCont> m_dataList;

};

//************************************************************************************************


typedef QList<QPair<QByteArray, RptContainer*> > BArrayList;

class ParamsContainerCommand : public QUndoCommand
{
public:
    ParamsContainerCommand(QList<PairCont> list, QUndoCommand *parent = 0);
    void undo();
    void redo();
    static BArrayList getBArrayFromContList(RptContainerList contList);
    static QList<PairCont> compoundArrays(BArrayList oldList, BArrayList newList);

private:
    bool m_create;
    QList<PairCont> m_dataList;

};

//************************************************************************************************

#endif // UNDOCOMMANDS_H
