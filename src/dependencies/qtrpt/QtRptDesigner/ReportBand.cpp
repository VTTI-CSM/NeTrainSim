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

#include "ReportBand.h"
#include "ui_ReportBand.h"
#include <QObject>
#include "mainwindow.h"

ReportBand::ReportBand(BandType type, int num)
{
    setFlag(QGraphicsItem::ItemIsMovable,false);
    m_menu = new QMenu;
    bandType = type;
    m_infocus = false;
    itemInTree = nullptr;
    bandNo = num;

    m_dsName = "";
    m_groupingField = "";
    m_startNewNumeration = false;
    m_showInGroup = false;
    m_startNewPage = false;
    m_groupLevel = 0;

    if (type == ReportTitle) {
        setText(QString("Report title %1").arg(num));
        setBandPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/reportTitle.png")));
    }
    if (type == ReportSummary) {
        setText(QString("Report summary %1").arg(num));
        setBandPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/reportFooter.png")));
    }
    if (type == PageHeader) {
        setText(QString("Page header %1").arg(num));
        setBandPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/pageTitle.png")));
    }
    if (type == PageFooter) {
        setText(QString("Page footer %1").arg(num));
        setBandPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/pageFooter.png")));
    }
    if (type == MasterData) {
        setText(QString("Master band %1").arg(num));
        setBandPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/data.png")));
    }
    if (type == MasterFooter) {
        setText(QString("Master footer %1").arg(num));
        setBandPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/dataFooter.png")));
    }
    if (type == MasterHeader) {
        setText(QString("Master header %1").arg(num));
        setBandPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/dataFooter.png")));
    }
    if (type == DataGroupHeader) {
        setText(QString("Data Group Header %1").arg(num));
        setBandPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/dataGroupingHeader.png")));
    }
    if (type == DataGroupFooter) {
        setText(QString("Data Group Footer %1").arg(num));
        setBandPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/dataGroupingFooter.png")));
    }
}

void ReportBand::setMenu(QMenu* menu)
{
    QIcon icon;
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/delete.png")), QIcon::Normal, QIcon::On);

    auto actBandDel = new QAction(tr("Delete"),this);
    actBandDel->setObjectName("actBandDel");
    actBandDel->setIcon(icon);
    QObject::connect(actBandDel, SIGNAL(triggered()), this, SIGNAL(itemRemoving()));

    m_menu->actions().clear();
    m_menu->addActions(menu->actions());
    m_menu->addAction(actBandDel);
}

void ReportBand::setHeight(qreal value)
{
    GraphicsBox::setHeight(titleHeight + value);
}

QString ReportBand::getGroupingField()
{
    return m_groupingField;
}

void ReportBand::setGroupingField(QString value)
{
    m_groupingField = value;
}

int ReportBand::getStartNewNumertaion()
{
    return m_startNewNumeration;
}

void ReportBand::setStartNewNumeration(bool value)
{
    m_startNewNumeration = value;
}

QString ReportBand::getDSName()
{
    return m_dsName;
}

void ReportBand::setDSName(QString value)
{
    m_dsName = value;
}

bool ReportBand::getShowInGroup()
{
    return m_showInGroup;
}

void ReportBand::setShowInGroup(bool value)
{
    m_showInGroup = value;
}

bool ReportBand::getStartNewPage()
{
    return m_startNewPage;
}

void ReportBand::setStartNewPage(bool value)
{
    m_startNewPage = value;
}

int ReportBand::getGroupLevel()
{
    return m_groupLevel;
}

void ReportBand::setGroupLevel(int value)
{
    m_groupLevel = value;

    if (bandType == DataGroupFooter)
        setText(QString("Data Group Footer %1 Level %2").arg(bandNo).arg(m_groupLevel));
}

void ReportBand::loadParamFromXML(QDomElement e)
{
    m_groupingField = e.attribute("groupingField");
    m_dsName = e.attribute("dataSourceName");
    m_startNewNumeration = e.attribute("startNewNumeration").toInt();
    m_showInGroup = e.attribute("showInGroup").toInt();
    m_startNewPage = e.attribute("startNewPage").toInt();
    m_groupLevel = e.attribute("groupLevel").toInt();

    setGroupLevel(m_groupLevel);
}

QDomElement ReportBand::saveParamToXML(QSharedPointer<QDomDocument> xmlDoc)
{
    QDomElement elem = xmlDoc->createElement("ReportBand");
    QString type;

    if (this->bandType == ReportTitle)
        type = "ReportTitle";
    if (this->bandType == ReportSummary)
        type = "ReportSummary";
    if (this->bandType == PageHeader)
        type = "PageHeader";
    if (this->bandType == PageFooter)
        type = "PageFooter";
    if (this->bandType == MasterData) {
        type = "MasterData";
        elem.setAttribute("dataSourceName",this->getDSName());
        elem.setAttribute("groupingField",this->getGroupingField());
        elem.setAttribute("startNewNumeration",this->getStartNewNumertaion());
        elem.setAttribute("startNewPage",this->getStartNewPage());
    }
    if (this->bandType == MasterFooter) {
        type = "MasterFooter";
        elem.setAttribute("showInGroup",this->getShowInGroup());
    }
    if (this->bandType == MasterHeader) {
        type = "MasterHeader";
        elem.setAttribute("showInGroup",this->getShowInGroup());
    }
    if (this->bandType == DataGroupHeader) {
        type = "DataGroupHeader";
    }
    if (this->bandType == DataGroupFooter) {
        type = "DataGroupFooter";
        elem.setAttribute("groupLevel", m_groupLevel);
        elem.setAttribute("showInGroup","1");
    }
    elem.setAttribute("name",this->objectName());
    elem.setAttribute("type",type);
    elem.setAttribute("bandNo",this->bandNo);
    //        elem.setAttribute("top",widget->geometry().y());
    //        elem.setAttribute("left",widget->geometry().x());
    elem.setAttribute("width",this->getWidth());
    elem.setAttribute("height",this->getHeight() - this->titleHeight);

    return elem;
}
