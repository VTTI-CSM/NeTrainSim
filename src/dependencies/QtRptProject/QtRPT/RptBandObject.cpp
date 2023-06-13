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

#include "RptBandObject.h"

/*!
 \class RptBandObject
 \inmodule RptBandObject.cpp
 \brief RptBandObject represent band of the report. On RptBandObject the RptFieldObject are placed

  The inner structure of report is following:
  \code
  QtRPT
    RptPageObject
      RptBandObject
        RptFieldObject
  \endcode
  There are possible to have some RptBandObject on the report's page but different type.
*/

/*!
 \variable RptBandObject::name
 \brief The name of band.
*/

/*!
 \variable RptBandObject::groupingField
 \brief The name of the field on which the grouping is performed. Used with DataGroupHeader band only.
*/

/*!
 \variable RptBandObject::showInGroup
 \brief The value of which is DataGroupHeader will be shown inside or outside of Data group.
  Used with DataGroupHeader band only.
*/

/*!
 \variable RptBandObject::startNewPage
 \brief The value of which is each group prints on different page.
  Used with DataGroupHeader band only.
*/

/*!
 \variable RptBandObject::startNewNumeration
 \brief The value of which is each group has own numeration.
  Used with DataGroupHeader band only.
*/

/*!
 \variable RptBandObject::realHeight
 \brief Real height of the band. Height is adjusted by QtRPT
 \sa height
*/

/*!
 \variable RptBandObject::height
 \brief Height of the band, setted by user.
 \sa realHeight
*/

/*!
 \variable RptBandObject::width
 \brief Width of the band.
*/

/*!
 \variable RptBandObject::left
 \brief Left position of the band.
*/

/*!
 \variable RptBandObject::right
 \brief Right position of the band.
*/

/*!
 \variable RptBandObject::type
 \brief Type of the band.
 \sa QtRptName::BandType
*/

/*!
 \variable RptBandObject::fieldList
 \brief List of fields which are placed on current band.
 \sa RptFieldObject
*/

/*!
 \variable RptBandObject::parentReportPage
 \brief pointer to parent report page.
 \sa RptPageObject
*/

/*!
 \fn RptBandObject::RptBandObject()
    Constructs a RptBandObject object.
*/
/*!
 \fn RptFieldObject::RptFieldObject()
    Constructs a RptFieldObject object.
*/
RptBandObject::RptBandObject(QObject *parent)
: QObject(parent)
{

}

void RptBandObject::setProperty(QtRPT *qtrpt, QDomElement docElem)
{
    name               = docElem.attribute("name");
    realHeight         = docElem.attribute("height").toInt();
    height             = docElem.attribute("height").toInt();
    width              = docElem.attribute("width").toInt();
    left               = docElem.attribute("left").toInt();
    right              = docElem.attribute("right").toInt();
    groupingField      = docElem.attribute("groupingField");
    groupLevel         = docElem.attribute("groupLevel", "0").toInt();
    showInGroup        = docElem.attribute("showInGroup","0").toInt();
    startNewNumeration = docElem.attribute("startNewNumeration","0").toInt();
    startNewPage       = docElem.attribute("startNewPage","0").toInt();
    bandNo             = docElem.attribute("bandNo", "1").toInt();
    dsName             = docElem.attribute("dataSourceName", "");
    sortDataInGroup    = true;
    groupHeaderEachlevel = false;

    if (docElem.attribute("type") == "ReportTitle")
        type = ReportTitle;
    if (docElem.attribute("type") == "PageHeader")
        type = PageHeader;
    if (docElem.attribute("type") == "MasterData")
        type = MasterData;
    if (docElem.attribute("type") == "PageFooter")
        type = PageFooter;
    if (docElem.attribute("type") == "ReportSummary")
        type = ReportSummary;
    if (docElem.attribute("type") == "MasterFooter")
        type = MasterFooter;
    if (docElem.attribute("type") == "MasterHeader")
        type = MasterHeader;
    if (docElem.attribute("type") == "DataGroupHeader")
        type = DataGroupHeader;
    if (docElem.attribute("type") == "DataGroupFooter")
        type = DataGroupFooter;

    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        auto fieldObject = new RptFieldObject(this);
        fieldObject->parentBand = this;
        fieldObject->setProperty(qtrpt,e);
        fieldList.append(fieldObject);

        n = n.nextSibling();
    }
}

/*!
 \fn void RptBandObject::addField(RptFieldObject *field)
    Add \a field to current RptBandObject object.

    \sa RptFieldObject
*/
void RptBandObject::addField(RptFieldObject *field)
{
    field->parentBand = this;
	field->m_qtrpt = this->m_qtrpt;
    fieldList.append(field);
}

/*!
 \fn RptBandObject *RptBandObject::clone()
    Clone the current band and return \c RptBandObject of the new band object
*/
RptBandObject *RptBandObject::clone()
{
    auto band = new RptBandObject(this->parent());
    band->name = "ReportTitle";
    band->height = this->height;
    band->type = this->type;
    band->m_qtrpt = this->m_qtrpt;

    for (auto &field : fieldList) {
        auto newField = field->clone();
        band->addField(newField);
    }

    return band;
}

void RptBandObject::setStartNewPage(bool value)
{
    startNewPage = value;
}

/*!
  Destroys the object, deleting all its child objects.
 */
RptBandObject::~RptBandObject()
{
    if (!fieldList.isEmpty())
        qDeleteAll(fieldList);
    fieldList.clear();
}

QDebug operator<<(QDebug dbg, const RptBandObject &obj)
{
    dbg << obj.name << obj.fieldList;
    return dbg;
}
