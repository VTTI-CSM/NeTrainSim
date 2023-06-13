/*
Name: XYZ
Version: 2.2.2
Web-site: https://qtrpt.sourceforge.io
Programmer: Aleksey Osipov
E-mail: aliks-os@ukr.net
Web-site: http://www.aliks-os.tk

Copyright 2012-2021 Aleksey Osipov

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

#include "XYZ_ParamTree.h"
#include <QDebug>

XYZParamTree::ParamTree(QWidget *parent)
: QTreeWidget(parent)
{
	addParam(1, "String1", "str1", XYZParamTree::ParamType::String);
	addParam(2, "String2", "str2", XYZParamTree::ParamType::String);
}


XYZParamTree::~XYZParamTree()
{}

void XYZParamTree::addParam(int paramID, QString paramName, QVariant value, ParamType paramType)
{
	QTreeWidgetItem *item = findItemInTree(paramName);
    QTreeWidgetItem *parentNode = nullptr;
	
	if (item == nullptr)
	{
        item = new QTreeWidgetItem(this);
		item->setData(1, Qt::UserRole, paramType);
		item->setData(1, Qt::UserRole+1, paramID);  // Should be unique for parameter
		item->setText(0, tr("Name"));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
        item->setText(1, value.toString());
	}
	else
	{
		//setParam(paramName, value);
	}
}

void XYZParamTree::setParam(QString paramName, QVariant value)
{
	QTreeWidgetItem *item = findItemInTree(paramName);
    QTreeWidgetItem *parentNode = nullptr;
	
	if (item == nullptr)
        item = new QTreeWidgetItem(this);
}

QTreeWidgetItem *XYZParamTree::findItemByName(QString paramName)
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        QTreeWidgetItem *item = (*it);
        if (item->data(1, Qt::UserRole) == paramName)
            return item;
        ++it;
    }
    return nullptr;
}