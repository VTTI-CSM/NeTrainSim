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

#include "RptPageObject.h"
#include "CommonClasses.h"

/*!
 \class RptPageObject
 \inmodule RptPageObject.cpp
 \brief The RptPageObject class allow to construct the QtRPT report from scrutch.
 RptPageObject represent report's page. On RptPageObject the RptBandObjects are placed

  The inner structure of report is following:
  \code
  QtRPT
    RptPageObject
      RptBandObject
        RptFieldObject
  \endcode
  There are possible to have some RptPageObject in the report.
  It allows combining several reports in one Report.
*/

/*!
 \variable RptPageObject::pageNo
 \brief The number of report's page.
*/

/*!
 \variable RptPageObject::orientation
 \brief The orientation of report's page.
*/

/*!
 \variable RptPageObject::ph
 \brief The height of report's page.
*/

/*!
 \variable RptPageObject::pw
 \brief The width of report's page.
*/

/*!
 \variable RptPageObject::ml
 \brief The left's margin of report's page.
*/

/*!
 \variable RptPageObject::mr
 \brief The girht's margin of report's page.
*/

/*!
 \variable RptPageObject::mt
 \brief The top's margin of report's page.
*/

/*!
 \variable RptPageObject::mb
 \brief The buttom's margin of report's page.
*/

/*!
 \variable RptPageObject::border
 \brief Sets draw or not border arround the page. Defualt is false.
*/

/*!
 \variable RptPageObject::borderWidth
 \brief The border's width of report's page. Default value is 1px.
*/

/*!
 \variable RptPageObject::borderColor
 \brief The border's color of report's page. Default is Qt::black
*/

/*!
 \variable RptPageObject::borderStyle
 \brief The border's style of report's page. Default value is "solid"
*/

/*!
 \fn RptPageObject::RptPageObject()
    Constructs a RptPageObject object.
*/
RptPageObject::RptPageObject(QtRPT *qtrpt)
: QObject(qtrpt)
{
    this->m_qtrpt = qtrpt;
    this->orientation = 0;
    this->ph = 1188;
    this->pw = 840;
    this->ml = 40;
    this->mr = 40;
    this->mt = 40;
    this->mb = 40;
    this->border = false;
    this->borderWidth = 1;
    this->borderColor = Qt::black;
    this->borderStyle = "solid";
    this->rptDsInline = nullptr;
    this->rtpSql = nullptr;
    this->sqlConnection.active = false;
    this->watermark = false;
    this->watermarkOpacity = 1;
    this->m_visible = true;
    this->m_totalPages = 0;
}

void RptPageObject::setProperty(QtRPT *qtrpt, QDomElement docElem)
{
    QByteArray byteArray = QByteArray::fromBase64(docElem.attribute("watermarkPixmap").toLatin1());

    ph                = docElem.attribute("pageHeight").toInt();
    pw                = docElem.attribute("pageWidth").toInt();
    ml                = docElem.attribute("marginsLeft").toInt();
    mr                = docElem.attribute("marginsRight").toInt();
    mt                = docElem.attribute("marginsTop").toInt();
    mb                = docElem.attribute("marginsBottom").toInt();
    orientation       = docElem.attribute("orientation").toInt();
    pageNo            = docElem.attribute("pageNo").toInt();
    border            = docElem.attribute("border").toInt();
    borderWidth       = docElem.attribute("borderWidth").toInt();
    borderColor       = colorFromString(docElem.attribute("borderColor"));
    borderStyle       = docElem.attribute("borderStyle");
    watermark         = docElem.attribute("watermark", "0").toInt();
    watermarkOpacity  = docElem.attribute("watermarkOpacity", "0.5").toDouble();
    watermarkPixmap   = QPixmap::fromImage(QImage::fromData(byteArray, "PNG"));



    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull() && e.tagName() == "ReportBand") {
            auto bandObject = new RptBandObject(this);
            bandObject->parentReportPage = this;
            bandObject->setProperty(qtrpt,e);
            bandList.append(bandObject);
        }
        n = n.nextSibling();
    }
}

/*!
 \fn void RptPageObject::addBand(RptBandObject *band)
    Add \a band to current RptBandObject object.

    \sa RptBandObject
*/
void RptPageObject::addBand(RptBandObject *band)
{
    band->parentReportPage = this;
	band->m_qtrpt = this->m_qtrpt;
    bandList.append(band);
}

/*!
 \fn RptBandObject RptPageObject::getBand(BandType type)
    Find and return \c RptBandObject on this report page the field by
    the given \a type.

    \sa RptBandObject
*/
RptBandObject *RptPageObject::getBand(BandType type, int No, int groupLevel)
{
    for (const auto &band : bandList)
        if (type == DataGroupFooter) {
            if (band->type == type && band->bandNo == No && band->groupLevel == groupLevel)
                return band;
        } else if (type == DataGroupHeader ||
                   type == MasterHeader ||
                   type == MasterData ||
                   type == MasterFooter ||
                   type == DataGroupFooter) {
            if (band->type == type && band->bandNo == No)
                return band;
        } else {
            if (band->type == type)
                return band;
        }

    return nullptr;
}

/*!
 \fn RptBandObject *RptPageObject::getBand(QString name)
    Find and return \c RptBandObject on this report page the band with
    the given \a name.

    \sa RptBandObject
*/
RptBandObject *RptPageObject::getBand(QString name)
{
    for (auto band : bandList)
        if (band->name == name)
            return band;

    return nullptr;
}

/*!
 \fn RptBandObject RptPageObject::bandsCountByType(BandType type)
    Returns count of the bands on this report page by the given \a type.

    \sa RptBandObject
*/
int RptPageObject::bandsCountByType(BandType type)
{
    int count = 0;
    for (const auto &band : bandList)
        if (band->type == type)
            if (band->bandNo > count)
                count = band->bandNo;

    return count;
}

/*!
 \fn RptFieldObject RptPageObject::findFieldObjectByName(QString name)
    Find and return \c RptFieldObject on this report page the field with
    the given \a name.

    \sa RptFieldObject
*/
RptFieldObject *RptPageObject::findFieldObjectByName(QString name)
{
    for (const auto &band : bandList)
        for (const auto &field : band->fieldList)
            if (field->objectName() == name)
                return field;

    return nullptr;
}

void RptPageObject::initCrossTabProcessedRows()
{
    for (const auto &band : bandList)
        for (const auto &field : band->fieldList)
            if (field->fieldType == CrossTab)
                field->crossTab->setProcessedCount(0);
}

int RptPageObject::crossTabParts()
{
    for (const auto &band : bandList)
        for (const auto &field : band->fieldList)
            if (field->fieldType == CrossTab)
                return field->crossTab->parts();

    return 0;
}

QList<RptFieldObject*> RptPageObject::crossTabs()
{
    QList<RptFieldObject*> fieldList;

    for (const auto &band : bandList)
        for (const auto &field : band->fieldList)
            if (field->fieldType == CrossTab)
                fieldList << field;

    return fieldList;
}

/*!
 \fn RptPageObject *RptPageObject::clone()
    Clone the current page and return \c RptPageObject of the new page object
*/
RptPageObject *RptPageObject::clone()
{
    auto reportPage = new RptPageObject(m_qtrpt);
    reportPage->pageNo      = pageNo;
    reportPage->orientation = orientation;
    reportPage->ph          = ph;
    reportPage->pw          = pw;
    reportPage->ml          = ml;
    reportPage->mr          = mr;
    reportPage->mt          = mt;
    reportPage->mb          = mb;
    reportPage->border      = border;
    reportPage->borderWidth = borderWidth;
    reportPage->borderColor = borderColor;
    reportPage->borderStyle = borderStyle;
    reportPage->recordCount = recordCount;
    reportPage->setVisible(this->m_visible);

    for (auto band : bandList) {
        auto newBand = band->clone();
        reportPage->addBand(newBand);
    }

    return reportPage;
}

/*!
 \fn bool RptPageObject::isVisible()
    Return mark that this page is visible (printable)

    \sa setVisible
*/
bool RptPageObject::isVisible()
{
    return m_visible;
}

/*!
 \fn void RptPageObject::setVisible(bool value)
    Set mark that this page is visible (printable)

    \sa isVisible
*/
void RptPageObject::setVisible(bool value)
{
    m_visible = value;
}

/*!
 \fn RptPageObject *RptPageObject::totalPages()
    Return the number of pages on this report page

    \sa setTotalPages
*/
quint16 RptPageObject::totalPages()
{
    return m_totalPages;
}

/*!
 \fn RptBandObject RptPageObject::setTotalPages(quint16 value)
    Set number of pages on this report page by the given \a value.
    For internal use only!.

    \sa totalPages
*/
void RptPageObject::setTotalPages(quint16 value)
{
    m_totalPages = value;
}

/*!
  Destroys the object, deleting all its child objects.
 */
RptPageObject::~RptPageObject()
{
    qDeleteAll(bandList);
    bandList.clear();
}

QDebug operator<<(QDebug dbg, const RptPageObject &obj)
{
    dbg << "Report #" << obj.pageNo  << obj.bandList;
    return dbg;
}
