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

#include "XmlViewModel.h"
#include <QDebug>

XMLViewModel::XMLViewModel(QDomDocument *xmlDoc, QObject *parent)
: QStandardItemModel(parent)
{
    QDomElement docElem = xmlDoc->documentElement();  //get root element
    insertChilds(docElem, this->invisibleRootItem());
}

void XMLViewModel::insertChilds(QDomNode node, QStandardItem *parent)
{
    if (node.hasAttributes()) {
        for (int t=0; t<node.attributes().count(); t++) {
            QString str = node.attributes().item(t).toAttr().name();
            str = str+" ("+node.attributes().item(t).toAttr().value()+")";

            QIcon icon(QPixmap(QString::fromUtf8(":/new/prefix1/images/xmlAttr.png")));
            QStandardItem *subItem = new QStandardItem(icon, str);
            subItem->setToolTip(node.attributes().item(t).toAttr().value());
            parent->appendRow(subItem);
        }
    }

    if (node.hasChildNodes()) {
        for (int t=0; t<node.childNodes().count(); t++) {
            QString str = node.childNodes().at(t).toElement().tagName();
            QIcon icon(QPixmap(QString::fromUtf8(":/new/prefix1/images/xmlTag.png")));

            QStandardItem *subItem = new QStandardItem(icon, str);
            parent->appendRow(subItem);

            insertChilds(node.childNodes().at(t), subItem);
        }
    }
}
