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

#include "RptFieldObject.h"
#include "CommonClasses.h"

/*!
 \class RptFieldObject
 \inmodule RptFieldObject.cpp
 \brief RptFieldObject represent field of the report. On RptBandObject the RptFieldObject are placed

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
 \variable RptFieldObject::name
 \brief The name of field.
*/

/*!
 \variable RptFieldObject::value
 \brief The value of field.
*/

/*!
 \variable RptFieldObject::rect
 \brief Rectangle of the field.
*/

/*!
 \variable RptFieldObject::borderTop
 \brief Top's border color.
 \sa borderColor
*/

/*!
 \variable RptFieldObject::borderBottom
 \brief Bottom's border color.
 \sa borderColor
*/

/*!
 \variable RptFieldObject::borderLeft
 \brief Left's border color.
 \sa borderColor
*/

/*!
 \variable RptFieldObject::borderRight
 \brief Right's border color.
 \sa borderColor
*/

/*!
 \variable RptFieldObject::borderColor
 \brief Common border color for all sides.
*/

/*!
 \variable RptFieldObject::fontColor
 \brief Current font color. Default is Qt::black
*/

/*!
 \variable RptFieldObject::backgroundColor
 \brief Current background color. Default is Qt::white
*/

/*!
 \variable RptFieldObject::borderWidth
 \brief Border's width. Default value is 1px.
*/

/*!
 \variable RptFieldObject::autoHeight
 \brief Auto height of field. Default value is false.
 \note Applicable only for fields which placed on MasterDataBand.
*/

/*!
 \variable RptFieldObject::textWrap
 \brief Text wrap of field. Default value is true.
 \note For type Text only.
*/

/*!
 \variable RptFieldObject::aligment
 \brief Aligment of text.
*/

/*!
 \variable RptFieldObject::borderStyle
 \brief Border's style.
 \list
   \li solid
   \li dashed
   \li dotted
   \li dot-dash
   \li dot-dot-dash
 \endlist
*/

/*!
 \variable RptFieldObject::font
 \brief Font of the Text field.
*/

/*!
 \variable RptFieldObject::fieldType
 \brief Type of the field.
 \sa QtRptName::FieldType
*/

/*!
 \variable RptFieldObject::formatString
 \brief Format of number value.
*/

/*!
 \variable RptFieldObject::highlighting
 \brief Encoded string containing the conditions of highlighting.
*/

/*!
 \variable RptFieldObject::imgFormat
 \brief Extension of image format.
 \note For type Image only.
*/

/*!
 \variable RptFieldObject::printing
 \brief Mark of printing.
*/

/*!
 \variable RptFieldObject::barcodeType
 \brief Type of the barcode.
 \note For type Barcode only.
*/

/*!
 \variable RptFieldObject::barcodeFrameType
 \brief Type of the barcode's frame.
 \list
   \li 0 - no border
   \li 1 - bind
   \li 2 - box
 \endlist
 \note For type Barcode only.
*/

/*!
 \variable RptFieldObject::barcodeHeight
 \brief Height of Barcode.
 \note For type Barcode only.
*/

/*!
 \variable RptFieldObject::ignoreAspectRatio
 \brief Ignore or not aspect ratio for image.
 \note For type Image only.
*/

/*!
 \variable RptFieldObject::picture
 \brief String, which must contains 84 bit base array.
 \note For type Image only.
*/

/*!
 \variable RptFieldObject::parentBand
 \brief Pointer to the parent band.
 \sa RptBandObject
*/

/*!
 \variable RptFieldObject::lineStartX
 \brief X of line start.
 \note For type Line only.
*/

/*!
 \variable RptFieldObject::lineEndX
 \brief X of line end.
 \note For type Line only.
*/

/*!
 \variable RptFieldObject::lineStartY
 \brief Y of line start.
 \note For type Line only.
*/

/*!
 \variable RptFieldObject::lineEndY
 \brief Y of line end.
 \note For type Line only.
*/

/*!
 \variable RptFieldObject::arrowStart
 \brief Draw arrow at the start.
 \note For type Line only.
*/

/*!
 \variable RptFieldObject::arrowEnd
 \brief Draw arrow at the end.
 \note For type Line only.
*/

/*!
 \variable RptFieldObject::showGrid
 \brief Show or not grid at diagram.
 \note For type Diagram only.
*/

/*!
 \variable RptFieldObject::showLegend
 \brief Show or not legend at diagram.
 \note For type Diagram only.
*/

/*!
 \variable RptFieldObject::showCaption
 \brief Show or not caption at diagram.
 \note For type Diagram only.
 \sa showGraphCaption
*/

/*!
 \variable RptFieldObject::showGraphCaption
 \brief Show or not legend at caption for each graph.
 \note For type Diagram only.
 \sa showCaption
*/

/*!
 \variable RptFieldObject::showPercent
 \brief Show persent value or show absolute.
 \note For type Diagram only.
*/

/*!
 \variable RptFieldObject::autoFillData
 \brief Automatic fill diagram from certian data or manual filling.
 \note For type Diagram only.
*/

/*!
 \variable RptFieldObject::caption
 \brief String represents the caption.
 \note For type Diagram only.
*/

/*!
 \variable RptFieldObject::graphList
 \brief List containing GraphParam which holds data for each Graph.
 \note For type Diagram only.
*/

/*!
 \fn int RptFieldObject::recNo()
    \c Return a current record number.
*/

/*!
 \fn int RptFieldObject::reportPage()
    \c Return a current Report page number.
*/

/*!
 \fn RptFieldObject::RptFieldObject()
    Constructs a RptFieldObject object.
*/
RptFieldObject::RptFieldObject(QObject *parent)
: QObject(parent)
{
    qRegisterMetaType<GraphDataList>("GraphDataList");

    this->highlighting = "";
    this->autoHeight = false;
    this->backgroundColor = Qt::white;
    this->m_backgroundColor = Qt::white;
    this->fontColor = Qt::black;
    this->m_fontColor = Qt::black;
    QFont font;
    font.setFamily(font.defaultFamily());
    font.setPointSize(12);
    this->font = font;
    this->printing = "1";
    this->rotate = 0;
    this->borderWidth = 1;
    this->borderColor = Qt::black;
    this->borderBottom = Qt::black;
    this->borderRight = Qt::black;
    this->borderLeft = Qt::black;
    this->borderTop = Qt::black;
    this->aligment = Qt::AlignVCenter | Qt::AlignLeft;
    this->parentBand = nullptr;
    this->parentCrossTab = nullptr;
    this->m_visible = true;
    this->renderingMode = 0;

    #if QT_VERSION >= 0x050800
        this->chart = nullptr;
    #endif
}

/*!
 \fn RptFieldObject::~RptFieldObject()
    Destructs a RptFieldObject object.
*/
RptFieldObject::~RptFieldObject()
{
    if (fieldType == CrossTab)
        delete crossTab;

//    if (chart != nullptr)
//        delete chart;
}

/*!
 \fn void RptFieldObject::setDefaultFontColor(QColor value)
    Sets default font color with \a value.
*/
void RptFieldObject::setDefaultFontColor(QColor value)
{
    fontColor = value;
    m_fontColor = value;
}

/*!
 \fn void RptFieldObject::setDefaultBackgroundColor(QColor value)
    Sets default background color with \a value.
*/
void RptFieldObject::setDefaultBackgroundColor(QColor value)
{
    backgroundColor = value;
    m_backgroundColor = value;
}

void RptFieldObject::setObjectName(const QString &objName)
{
    name = objName;
    QObject::setObjectName(objName);
}

void RptFieldObject::setProperty(QtRPT *qtrpt, QDomElement e)
{
    m_qtrpt = qtrpt;
    highlighting = e.attribute("highlighting","");
    printing = e.attribute("printing","1");
    renderingMode = e.attribute("renderingMode","0").toInt();
    this->setObjectName(e.attribute("name"));
    value = e.attribute("value").replace("&Acirc","");
    rect.setX(e.attribute("left").toInt());
    rect.setY(e.attribute("top").toInt());
    rect.setWidth(e.attribute("width").toInt());
    rect.setHeight(e.attribute("height").toInt());
    borderTop = colorFromString(e.attribute("borderTop"));
    borderBottom = colorFromString(e.attribute("borderBottom"));
    borderLeft  = colorFromString(e.attribute("borderLeft"));
    borderRight = colorFromString(e.attribute("borderRight"));
    borderWidth = e.attribute("borderWidth","1").replace("px","").toInt();
    borderStyle = e.attribute("borderStyle","solid");
    borderColor = colorFromString(e.attribute("borderColor"));

    rotate = e.attribute("rotate","0").toInt();

    aligment = QtRPT::getAligment(e);
    autoHeight = e.attribute("autoHeight","0").toInt();

    QFont m_font(e.attribute("fontFamily"), e.attribute("fontSize").toInt());
    m_font.setBold(e.attribute("fontBold").toInt());
    m_font.setItalic(e.attribute("fontItalic").toInt());
    m_font.setUnderline(e.attribute("fontUnderline").toInt());
    m_font.setStrikeOut(e.attribute("fontStrikeout").toInt());

    font = m_font;

    backgroundColor = colorFromString(e.attribute("backgroundColor"));
    m_backgroundColor = backgroundColor;
    fontColor = colorFromString(e.attribute("fontColor"));
    m_fontColor = fontColor;

    imgFormat = e.attribute("imgFormat","PNG");
    textWrap = e.attribute("textWrap","1").toInt();

    fieldType = qtrpt->getFieldType(e);
    formatString = e.attribute("format","");
    inputFormatString = e.attribute("input_format","");

    barcodeType = e.attribute("barcodeType","13").toInt();
    barcodeFrameType = e.attribute("barcodeFrameType","0").toInt();
    barcodeHeight = e.attribute("barcodeHeight","50").toInt();

    picture = QByteArray::fromBase64(e.attribute("picture","text").toLatin1());
    ignoreAspectRatio = e.attribute("ignoreAspectRatio","1").toInt();

    lineStartX = e.attribute("lineStartX").toInt();
    lineEndX = e.attribute("lineEndX").toInt();
    lineStartY = e.attribute("lineStartY").toInt();
    lineEndY = e.attribute("lineEndY").toInt();
    arrowStart = e.attribute("arrowStart","0").toInt();
    arrowEnd = e.attribute("arrowEnd","0").toInt();

    if (fieldType == Diagram) {
        #if QT_VERSION >= 0x050800
            chart = new QChart();

            chart->layout()->setContentsMargins(0, 0, 0, 0);
            chart->setContentsMargins(0,0,0,0);
            chart->setMargins(QMargins());
            chart->setBackgroundRoundness(0);

            chart->setTitle(e.attribute("caption"));
            chart->legend()->setVisible(e.attribute("showLegend", "1").toInt());
            chart->setProperty("staticChart", e.attribute("staticChart", "1").toInt());

            QFont fnt =  chart->titleFont();
            fnt.fromString(e.attribute("titleFont", chart->titleFont().toString()));
            chart->setTitleFont(fnt);

            fnt =  chart->legend()->font();
            fnt.fromString(e.attribute("legendFont", chart->legend()->font().toString()));
            chart->legend()->setFont(fnt);

            QColor color = colorFromString(e.attribute("colorLegend"));
            QBrush brush = chart->legend()->labelBrush();
            brush.setColor(color);
            chart->legend()->setLabelBrush(brush);

            color = colorFromString(e.attribute("colorBackground"));
            brush = chart->backgroundBrush();
            brush.setColor(color);
            brush.setStyle(Qt::SolidPattern);
            chart->setBackgroundBrush(brush);

            color = colorFromString(e.attribute("colorTitle"));
            brush = chart->titleBrush();
            brush.setColor(color);
            chart->setTitleBrush(brush);


            Qt::Alignment alig;
            if (e.attribute("legendAligment").toInt() == 0) alig = Qt::AlignTop;
            if (e.attribute("legendAligment").toInt() == 1) alig = Qt::AlignBottom;
            if (e.attribute("legendAligment").toInt() == 2) alig = Qt::AlignLeft;
            if (e.attribute("legendAligment").toInt() == 3) alig = Qt::AlignRight;
            chart->legend()->setAlignment(alig);


            if (e.attribute("chartType").contains("SeriesTypeLine")) {
                QDomNode c = e.firstChild();
                while(!c.isNull()) {
                    QDomElement graphElement = c.toElement();
                    if (!graphElement.isNull()) {
                        auto series = new QLineSeries();
                        series->setName(graphElement.attribute("caption"));
                        series->setColor(colorFromString(graphElement.attribute("color")));
                        series->setProperty("graphDS", graphElement.attribute("graphDS"));

                        QDomNode v = graphElement.firstChild();
                        while(!v.isNull()) {
                            QDomElement valueElement = v.toElement();
                            series->append(valueElement.attribute("x").toDouble(),
                                           valueElement.attribute("y").toDouble());

                            v = v.nextSibling();
                        }

                        chart->addSeries(series);
                    }

                    c = c.nextSibling();
                }
            }

            if (e.attribute("chartType") == "SeriesTypeBar" ||
                e.attribute("chartType") == "SeriesTypeStackedBar") {

                QAbstractSeries *abstrSeries = nullptr;

                if (e.attribute("chartType") == "SeriesTypeBar") {
                    auto series = new QBarSeries();
                    abstrSeries = series;
                }
                if (e.attribute("chartType") == "SeriesTypeStackedBar") {
                    auto series = new QStackedBarSeries();
                    abstrSeries = series;
                }


                QDomNode c = e.firstChild();
                while(!c.isNull()) {
                    QDomElement graphElement = c.toElement();
                    if (!graphElement.isNull()) {
                        auto barSet = new QBarSet(graphElement.attribute("caption"));
                        barSet->setColor(colorFromString(graphElement.attribute("color")));
                        barSet->setProperty("graphDS", graphElement.attribute("graphDS"));

                        QDomNode v = graphElement.firstChild();
                        while(!v.isNull()) {
                            QDomElement valueElement = v.toElement();
                            barSet->append(valueElement.attribute("val").toDouble());

                            v = v.nextSibling();
                        }



                        if (abstrSeries->type() == QAbstractSeries::SeriesTypeStackedBar) {
                            auto series = qobject_cast<QStackedBarSeries*>(abstrSeries);
                            series->append(barSet);
                        }
                        if (abstrSeries->type() == QAbstractSeries::SeriesTypeBar) {
                            auto series = qobject_cast<QBarSeries*>(abstrSeries);
                            series->append(barSet);
                        }
                    }

                    c = c.nextSibling();
                }

                chart->addSeries(abstrSeries);
                chart->createDefaultAxes();
                chart->update();
            }

            if (e.attribute("chartType") == "SeriesTypePie") {
                auto series = new QPieSeries();
                series->setName(e.attribute("caption"));
                series->setHoleSize(e.attribute("holeSize", "0.00").toDouble());
                series->setProperty("graphDS", e.attribute("graphDS"));

                chart->addSeries(series);

                QDomNode c = e.firstChild();
                while(!c.isNull()) {
                    QDomElement graphElement = c.toElement();
                    if (!graphElement.isNull()) {
                        auto slice = new QPieSlice(graphElement.attribute("caption"),
                                                   graphElement.attribute("value").toDouble(), series);
                        slice->setExploded(graphElement.attribute("sliceExploaded").toInt());
                        slice->setLabelVisible(graphElement.attribute("labelVisible").toInt());

                        series->append(slice);

                        QColor color = colorFromString(graphElement.attribute("color"));
                        slice->setColor(color);
                    }
                    c = c.nextSibling();
                }
            }

            chart->createDefaultAxes();
        #endif
    }

    if (fieldType == CrossTab) {
        crossTab = new RptCrossTabObject();
        crossTab->rect = this->rect;
        crossTab->parentField = this;
        crossTab->loadParamFromXML(e);
    }
}

void RptFieldObject::setWidth(int value)
{
    rect.setWidth(value);
}

int RptFieldObject::getWidth()
{
    return rect.width();
}

void RptFieldObject::setHeight(int value)
{
    rect.setHeight(value);
}

int RptFieldObject::getHeight()
{
    return rect.height();
}

void RptFieldObject::setTop(int value)
{
    rect.setY(value);
}

int RptFieldObject::getTop()
{
    return rect.y();
}

void RptFieldObject::setLeft(int value)
{
    rect.setX(value);
}

int RptFieldObject::getLeft()
{
    return rect.x();
}

void RptFieldObject::setChartData(GraphDataList dataList)
{
    Q_UNUSED(dataList);

//    struct GraphValue {
//        QString caption;  //for Pie, for Line - ignore
//        double valueX;    //for Line only
//        double valueY;
//    };

//    struct GraphData {
//        QList<GraphValue> valueList;
//        QString graphDS;
//        QString caption;
//    };

    #if QT_VERSION >= 0x050800
        if (chart->property("staticChart").toInt() == true)
            return;

        if (chart->series().at(0)->type() == QAbstractSeries::SeriesTypeBar ||
            chart->series().at(0)->type() == QAbstractSeries::SeriesTypeStackedBar) {
            if (chart->series().at(0)->type() == QAbstractSeries::SeriesTypeStackedBar) {
                auto series = qobject_cast<QStackedBarSeries*>(chart->series().at(0));
                series->clear();
            }
            if (chart->series().at(0)->type() == QAbstractSeries::SeriesTypeBar) {
                auto series = qobject_cast<QBarSeries*>(chart->series().at(0));
                series->clear();
            }
        }

        for (int i = 0; i < dataList.size(); i++) {
            if (chart->series().at(0)->type() == QAbstractSeries::SeriesTypeLine) {
                auto series = qobject_cast<QLineSeries*>(chart->series()[i]);
                series->clear();

                for (const auto &value : dataList.at(i).valueList)
                    series->append(value.valueX, value.valueY);

                chart->removeSeries(series);
                chart->addSeries(series);
            }
            if (chart->series().at(0)->type() == QAbstractSeries::SeriesTypeBar ||
                chart->series().at(0)->type() == QAbstractSeries::SeriesTypeStackedBar) {

                auto barSet = new QBarSet(dataList.at(i).caption);
                barSet->setColor(dataList.at(i).color);
                for (const auto &value : dataList.at(i).valueList)
                    barSet->append(value.valueY);

                if (chart->series().at(0)->type() == QAbstractSeries::SeriesTypeStackedBar) {
                    auto series = qobject_cast<QStackedBarSeries*>(chart->series().at(0));
                    series->append(barSet);
                }
                if (chart->series().at(0)->type() == QAbstractSeries::SeriesTypeBar) {
                    auto series = qobject_cast<QBarSeries*>(chart->series().at(0));
                    series->append(barSet);
                }
            }
            if (chart->series().at(0)->type() == QAbstractSeries::SeriesTypePie) {
                auto series = qobject_cast<QPieSeries*>(chart->series().at(0));
                series->clear();

                for (const auto &value : dataList.at(i).valueList) {
                    auto slice = new QPieSlice(value.caption, value.valueY, series);
                    series->append(slice);
                }

                chart->removeSeries(series);
                chart->addSeries(series);
            }
        }

        auto series = chart->series().at(0);
        chart->removeSeries(series);
        chart->addSeries(series);
    #endif
}

GraphDataList RptFieldObject::getChartData()
{
    GraphDataList dataList;

    #if QT_VERSION >= 0x050800
        if (chart->series().size() > 0) {
            if (chart->series().at(0)->type() == QAbstractSeries::SeriesTypeLine) {
                for (auto &absSeries : chart->series()) {
                    auto series = qobject_cast<QLineSeries*>(absSeries);

                    GraphData graphData;
                    graphData.color   = series->color();
                    graphData.graphDS = series->property("graphDS").toString();
                    graphData.caption = series->name();

                    QList<GraphValue> valueList;
                    for (const auto &point : series->points()) {
                        GraphValue value;
                        value.valueX = point.x();
                        value.valueY = point.y();
                        valueList << value;
                    }
                    graphData.valueList = valueList;

                    dataList << graphData;
                }
            }
            if (chart->series().at(0)->type() == QAbstractSeries::SeriesTypeBar ||
                chart->series().at(0)->type() == QAbstractSeries::SeriesTypeStackedBar) {

                QList<QBarSet*> barSets;

                if (chart->series().at(0)->type() == QAbstractSeries::SeriesTypeBar) {
                    auto series = qobject_cast<QBarSeries*>(chart->series().at(0));
                    barSets = series->barSets();
                }
                if (chart->series().at(0)->type() == QAbstractSeries::SeriesTypeStackedBar) {
                    auto series = qobject_cast<QStackedBarSeries*>(chart->series().at(0));
                    barSets = series->barSets();
                }

                for (const auto &barSet : barSets) {
                    GraphData graphData;
                    graphData.color   = barSet->color();
                    graphData.graphDS = barSet->property("graphDS").toString();
                    graphData.caption = barSet->label();

                    QList<GraphValue> valueList;
                    for (int row = 0; row < barSet->count(); row++) {
                        GraphValue value;
                        value.valueY = barSet->at(row);
                        valueList << value;
                    }
                    graphData.valueList = valueList;

                    dataList << graphData;
                }
            }
            if (chart->series().at(0)->type() == QAbstractSeries::SeriesTypePie) {
                auto series = qobject_cast<QPieSeries*>(chart->series().at(0));

                GraphData graphData;
                graphData.graphDS = series->property("graphDS").toString();
                graphData.caption = "";

                QList<GraphValue> valueList;
                for (const auto &slice : series->slices()) {
                    GraphValue value;
                    value.caption = slice->label();
                    value.valueY = slice->value();
                    valueList << value;
                }
                graphData.valueList = valueList;

                dataList << graphData;
            }
        }
    #endif

    return dataList;
}

void RptFieldObject::updateHighlightingParam()
{
    QFont m_font(font);
    m_font.setBold(m_qtrpt->processHighligthing(this, FntBold).toInt());
    m_font.setItalic(m_qtrpt->processHighligthing(this, FntItalic).toInt());
    m_font.setUnderline(m_qtrpt->processHighligthing(this, FntUnderline).toInt());
    m_font.setStrikeOut(m_qtrpt->processHighligthing(this, FntStrikeout).toInt());
    font = m_font;

    backgroundColor = colorFromString(m_qtrpt->processHighligthing(this, BgColor).toString());
    fontColor = colorFromString(m_qtrpt->processHighligthing(this, FntColor).toString());
}

/*!
 \fn RptFieldObject *RptFieldObject::clone()
    Clone the current field and return \c RptFieldObject of the new field object
*/
RptFieldObject *RptFieldObject::clone()
{
    auto field = new RptFieldObject(this->parent());
    field->setObjectName(this->objectName());
    field->value = value;
    field->rect = rect;
    field->borderTop = borderTop;
    field->borderBottom = borderBottom;
    field->borderLeft = borderLeft;
    field->borderRight = borderRight;
    field->borderColor = borderColor;
    field->fontColor = fontColor;
    field->backgroundColor = backgroundColor;
    field->autoHeight = autoHeight;


    field->borderWidth = borderWidth;
    field->autoHeight = autoHeight;
    field->textWrap = textWrap;
    field->rotate = rotate;

    field->aligment = aligment;
    field->borderStyle = borderStyle;
    field->font = font;
    field->fieldType = fieldType;
    field->formatString = formatString;
    field->inputFormatString = inputFormatString;
    field->highlighting = highlighting;
    field->imgFormat = imgFormat;
    field->printing = printing;
    field->barcodeType = barcodeType;
    field->barcodeFrameType = barcodeFrameType;
    field->barcodeHeight = barcodeHeight;
    field->ignoreAspectRatio = ignoreAspectRatio;
    field->picture = picture;
    field->parentBand = parentBand;
    //field->*parentCrossTab = parentCrossTab;

    field->lineStartX = lineStartX;
    field->lineEndX = lineEndX;
    field->lineStartY = lineStartY;
    field->lineEndY = lineEndY;
    field->arrowStart = arrowStart;
    field->arrowEnd = arrowEnd;

    field->m_fontColor = m_fontColor;
    field->m_backgroundColor = m_backgroundColor;
    field->m_recNo = m_recNo;
    field->m_reportPage = m_reportPage;
    field->m_top = m_top;
    field->m_qtrpt = m_qtrpt;


    return field;
}

/*!
 \fn RptFieldObject::isCrossTabChild()
    Return true if the field is a part of CrossTabObject.
*/
bool RptFieldObject::isCrossTabChild()
{
    return this->parentCrossTab != nullptr;
}

/*!
 \fn void RptFieldObject::setTop(int top)
    Sets \a top of field.

    \sa rect
*/
void RptFieldObject::setHTMLTop(int top)
{
    m_top = top;
}

/*!
 \fn QString RptFieldObject::getHTMLStyle()
    Return HTML representation of the field.
*/
QString RptFieldObject::getHTMLStyle()
{
    QString style;

    QString alig;
    if (this->aligment &Qt::AlignRight) alig = "right";
    if (this->aligment &Qt::AlignLeft) alig = "left";
    if (this->aligment &Qt::AlignHCenter) alig = "center";
    if (this->aligment &Qt::AlignJustify) alig = "justify";

    if (this->autoHeight == 1)
        this->rect.setHeight(parentBand->realHeight);

    if (fieldType == Text) {
        style = "style='color:"+this->fontColor.name()+";"+
                "background:"+this->backgroundColor.name()+";"+
                "border-left: solid thin "+this->borderLeft.name()+";"+
                "border-right: solid thin "+this->borderRight.name()+";"+
                "border-top: solid thin "+this->borderTop.name()+";"+
                "border-bottom: solid thin "+this->borderBottom.name()+";"+
                "text-align:"+alig+";"+
                "font-size:"+QString::number(font.pointSize()+5)+"px;"+
                "position: absolute; left: "+QString::number(this->rect.x())+";"+
                "top: "+QString::number(this->m_top)+";"+
                "width: "+QString::number(this->rect.width())+";"+
                "height: "+QString::number(this->rect.height())+";";
        if (font.bold() == true)
            style += "font-weight: bold;";
        if (font.italic() == true)
            style += "font-style: italic;";
        style += "'";
    }

    if (fieldType == TextImage || fieldType == Image || fieldType == DatabaseImage) {
        double dblAspectRatio = 0;
        int nHeight = this->rect.height();
        int nWidth = this->rect.height();

        if (this->rect.height())
            dblAspectRatio = (double)this->rect.width() / (double)this->rect.height();

        if (dblAspectRatio) {
            nWidth = ((int)rint(nHeight * dblAspectRatio)) & -3;

            if (nWidth > this->rect.width()) {
                nWidth = this->rect.width();
                nHeight = ((int)rint(nWidth / dblAspectRatio)) & -3;
            }
        }

        style = "<div style='"
                "position: absolute; left: "+QString::number(this->rect.x())+";"+
                "top: "+QString::number(this->m_top)+";"+
                "width: "+QString::number(this->rect.width())+";"+
                "height: "+QString::number(this->rect.height())+";"+
                "text-align:"+alig+";"+
                "'>"+
                "<img "
                "width="+QString::number(nWidth)+" "
                "height="+QString::number(nHeight)+" "+
                "src=\"data:image/png;base64,"+this->picture.toBase64()+"\" /></div>\n";
    }

    return style;
}

/*!
 \fn bool RptFieldObject::isVisible()
    Return mark that this page is visible (printable)

    \sa setVisible
*/
bool RptFieldObject::isVisible()
{
    return m_visible;
}

/*!
 \fn void RptFieldObject::setVisible(bool value)
    Set mark that this page is visible (printable)

    \sa isVisible
*/
void RptFieldObject::setVisible(bool value)
{
    m_visible = value;
}
