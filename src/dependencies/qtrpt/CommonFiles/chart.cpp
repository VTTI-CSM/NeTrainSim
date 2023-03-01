/*
Name: QtRpt
Version: 2.0.1
Web-site: http://www.qtrpt.tk
Programmer: Aleksey Osipov
E-mail: aliks-os@ukr.net
Web-site: http://www.aliks-os.tk

Copyright 2012-2017 Aleksey Osipov

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

#include "chart.h"

Chart::Chart(QWidget *parent)
: QWidget(parent)
{
    m_drawGrid = false;
    m_drawHistory = true;
    m_drawCaption = true;
    m_drawGraphCaption = true;
    m_showPercent = false;
    m_autoFillData = false;
    m_koefRes_h = 1;
    m_koefRes_w = 1;
    m_left = 0;
    m_top = 0;

    GraphParam param;
    QColor color;

    color = colorFromString("rgba(255,255,0,255)");
    param.color = color;
    param.valueReal = 150;
    param.caption = "Graph 1";
    setData(param);

    color = colorFromString("rgba(0,0,255,255)");
    param.color = color;
    param.valueReal = 70;
    param.caption = "Graph 2";
    setData(param);

    color = colorFromString("rgba(255,0,0,255)");
    param.color = color;
    param.valueReal = 220;
    param.caption = "Graph 3";
    setData(param);

    color = colorFromString("rgba(0,128,128,255)");
    param.color = color;
    param.valueReal = 30;
    param.caption = "Graph 4";
    setData(param,150);

    setParams(true, true, true, true, true, "Example 1", false);

    qRegisterMetaType<GraphParam>("GraphParam");
    qRegisterMetaType<GraphParamList>("GraphParamList");

    qRegisterMetaTypeStreamOperators<GraphParam>("GraphParam");
    qRegisterMetaTypeStreamOperators<GraphParamList>("GraphParamList");
}

void Chart::setKoef(float koefRes_w, float koefRes_h, float left, float top)
{
    m_koefRes_w = koefRes_w;
    m_koefRes_h = koefRes_h;
    m_left = left;
    m_top = top;
}

void Chart::loadXML(QDomElement e)
{
    QDomNode g = e.firstChild();
    if (e.hasChildNodes())
        clearData();

    while(!g.isNull()) {
        QDomElement ge = g.toElement(); // try to convert the node to an element.

        GraphParam param;
        param.color = colorFromString( ge.attribute("color") );
        param.valueReal = ge.attribute("value").toFloat();
        param.valueString = ge.attribute("value");
        param.caption = ge.attribute("caption");
        setData(param);

        g = g.nextSibling();
    }
}

void Chart::clearData()
{
    listOfGraph.clear();
}

void Chart::setData(GraphParam param, float value100Percent)
{
    listOfGraph.append(param);
    float maxValue = 0;
    for (int i=0; i < listOfGraph.size(); i++) {
        if (maxValue < listOfGraph.at(i).valueReal)
            maxValue = listOfGraph.at(i).valueReal;
    }

    for (int i=0; i < listOfGraph.size(); i++) {
        if (value100Percent == 0) {
            listOfGraph[i].valuePercent = listOfGraph.at(i).valueReal / maxValue;
            m_correctValue = 1;
        } else {
            listOfGraph[i].valuePercent = listOfGraph.at(i).valueReal / value100Percent;
            m_correctValue = value100Percent / maxValue;
        }
    }
}

void Chart::setParams(bool drawGrid,
                      bool drawHistory,
                      bool drawCaption,
                      bool drawGraphCaption,
                      bool showPercent,
                      QString caption,
                      bool autoFillData)
{
    m_caption = caption;
    m_drawGrid = drawGrid;
    m_drawHistory = drawHistory;
    m_drawCaption = drawCaption;
    m_drawGraphCaption = drawGraphCaption;
    m_showPercent = showPercent;
    m_autoFillData = autoFillData;

    if (drawHistory)
        m_rightSpaceWidth = 110;
    else
        m_rightSpaceWidth = 30;

    if (m_drawGraphCaption) {
        //m_bottomSpaceHeight = 30;
    } else {
        //m_bottomSpaceHeight = 10;
    }

    m_bottomSpaceHeight = 10;    
}

GraphParamList Chart::getGraphParamList()
{
    return listOfGraph;
}

QVariant Chart::getParam(ChartParam param)
{
    switch(param) {
        case DrawGrid:
            return m_drawGrid;
            break;
        case ShowCaption:
            return m_drawCaption;
            break;
        case ShowLegend:
            return m_drawHistory;
            break;
        case ShowGraphCaption:
            return m_drawGraphCaption;
            break;
        case ShowPercent:
            return m_showPercent;
            break;
        case Caption:
            return m_caption;
            break;
        case AutoFillData:
            return m_autoFillData;
            break;
        default: return QVariant();
    }
}

void Chart::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event);

    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    auto widget = qobject_cast<QWidget *>(this);
    QPainter painter(widget);
    paintChart(&painter);
}

void Chart::paintChart(QPainter *painter)
{
    int captionHeight = 50 * m_koefRes_h;
    int leftSpaceWidth = 20 * m_koefRes_w;
    m_rightSpaceWidth = m_rightSpaceWidth * m_koefRes_w;
    m_bottomSpaceHeight = m_bottomSpaceHeight * m_koefRes_h;

    QFont font(this->font());
    int cor = QFontMetrics(font).height() * m_koefRes_h;
    if (m_drawCaption) {
        font.setBold(true);
        //font.setItalic(processHighligthing(e, FontItalic).toInt());
        painter->setFont(font);

        QRectF textRect(m_left, m_top, this->width(), captionHeight);
        textRect.translate(0, cor );

        painter->drawText(textRect,Qt::AlignHCenter|Qt::AlignTop,m_caption);

        font.setBold(false);
        painter->setFont(font);
    }

    int heightOfGraph = (this->height() - captionHeight - m_bottomSpaceHeight) * m_correctValue;
    if (!listOfGraph.isEmpty()) {
        int widthOfGraph = (this->width()-m_rightSpaceWidth-leftSpaceWidth) / listOfGraph.size();

        //Draw graps
        for (int i=0; i < listOfGraph.size(); i++) {
            painter->fillRect(widthOfGraph*i+1+leftSpaceWidth+m_left,        //x
                             this->height()-1-m_bottomSpaceHeight + m_top,   //y
                             widthOfGraph,                                   //width
                             heightOfGraph*listOfGraph.at(i).valuePercent*-1 ,
                             listOfGraph.at(i).color);

            if (m_drawHistory) {
                QRectF textRect(m_left+this->width()-m_rightSpaceWidth+20*m_koefRes_w,
                                m_top+(100 + (i+1)*20)*m_koefRes_h,
                                m_rightSpaceWidth,
                                20*m_koefRes_h); //x y w h
                QRectF pointRect(m_left+this->width()-m_rightSpaceWidth+25*m_koefRes_w,
                                 m_top+(103 + (i+1)*20)*m_koefRes_h,
                                 10*m_koefRes_w,
                                 10*m_koefRes_h); //x y w h
                painter->drawText(textRect,Qt::AlignLeft,"         "+listOfGraph.at(i).caption);
                //painter.drawRect(textRect);//
                painter->fillRect(pointRect, listOfGraph.at(i).color);
            }
        }

        //Draw graph's caption
        if (m_drawGraphCaption) {
            for (int i=0; i < listOfGraph.size(); i++) {
                QString txt;
                if (m_showPercent)
                    txt = QString::number(listOfGraph.at(i).valuePercent*100,'f',1)+"%";
                else
                    txt = QString::number(listOfGraph.at(i).valueReal);

                QRectF textRect(m_left + widthOfGraph*i+1+leftSpaceWidth,   //x
                                m_top + heightOfGraph*listOfGraph.at(i).valuePercent*-1 + this->height()-20*m_koefRes_h-m_bottomSpaceHeight,
                                widthOfGraph+5*m_koefRes_w,                      //width
                                20*m_koefRes_h); //x y w h
                //painter->drawRect(textRect);
                painter->drawText(textRect,Qt::AlignCenter,txt);
            }
        }
    }

    //draw line X
    painter->drawLine(m_left + leftSpaceWidth,
                     m_top +this->height()-1-m_bottomSpaceHeight,
                     m_left + this->width()-m_rightSpaceWidth+20*m_koefRes_w,
                     m_top + this->height()-1-m_bottomSpaceHeight);
    painter->drawText(m_left + this->width()-m_rightSpaceWidth+30*m_koefRes_w,
                     m_top + this->height()-1-m_bottomSpaceHeight+5*m_koefRes_h,
                     "X");

    //draw line Y
    painter->drawLine(m_left + leftSpaceWidth,
                     m_top + this->height()-1-m_bottomSpaceHeight,
                     m_left + leftSpaceWidth,
                     m_top + captionHeight-10*m_koefRes_h
                     );
    painter->drawText(m_left + leftSpaceWidth-2*m_koefRes_w,
                     m_top + captionHeight-15*m_koefRes_h,
                     "Y");

    //draw grid
    if (m_drawGrid)
        for (int i=4; i > 0; i--) {
            painter->drawLine(m_left + leftSpaceWidth-5,
                             m_top + this->height()-2*m_koefRes_h-m_bottomSpaceHeight - heightOfGraph/4*i,
                             m_left + this->width()-m_rightSpaceWidth+5,
                             m_top + this->height()-2*m_koefRes_h-m_bottomSpaceHeight - heightOfGraph/4*i);
        }
}

void Chart::setProperties()
{
    this->setProperty("m_caption",m_caption);
    this->setProperty("m_drawGrid",m_drawGrid);
    this->setProperty("m_drawHistory",m_drawHistory);
    this->setProperty("m_drawCaption",m_drawCaption);
    this->setProperty("m_drawGraphCaption",m_drawGraphCaption);
    this->setProperty("m_showPercent",m_showPercent);
    this->setProperty("m_autoFillData",m_autoFillData);
    this->setProperty("m_rightSpaceWidth",m_rightSpaceWidth);
    this->setProperty("m_bottomSpaceHeight",m_bottomSpaceHeight);
    this->setProperty("m_captionHeight",m_captionHeight);
    this->setProperty("m_correctValue",m_correctValue);
    this->setProperty("m_koefRes_h",m_koefRes_h);
    this->setProperty("m_koefRes_w",m_koefRes_w);
    this->setProperty("m_left",m_left);
    this->setProperty("m_top",m_top);

    QVariant v = QVariant::fromValue<GraphParamList>(listOfGraph);
    this->setProperty("listOfGraph",v);
}

static QDataStream &operator<<(QDataStream &out, const GraphParam &t)
{
    out << t.color << t.valuePercent << t.valueReal << t.caption << t.valueString;
    return out;
}

static QDataStream &operator>>(QDataStream &in, GraphParam &t)
{
    in >> t.color >> t.valuePercent >> t.valueReal >> t.caption >> t.valueString;
    return in;
}

//Restore fields from properties
void Chart::setParamFromProperties()
{
    m_caption = this->property("m_caption").toString();
    m_drawGrid = this->property("m_drawGrid").toBool();
    m_drawHistory = this->property("m_drawHistory").toBool();
    m_drawCaption = this->property("m_drawCaption").toBool();
    m_drawGraphCaption = this->property("m_drawGraphCaption").toBool();
    m_showPercent = this->property("m_showPercent").toBool();
    m_autoFillData = this->property("m_autoFillData").toBool();
    m_rightSpaceWidth = this->property("m_rightSpaceWidth").toInt();
    m_bottomSpaceHeight = this->property("m_bottomSpaceHeight").toInt();
    m_captionHeight = this->property("m_captionHeight").toInt();
    m_correctValue = this->property("m_correctValue").toFloat();
    m_koefRes_h = this->property("m_koefRes_h").toFloat();
    m_koefRes_w = this->property("m_koefRes_w").toFloat();
    m_left = this->property("m_left").toFloat();
    m_top = this->property("m_top").toFloat();
    listOfGraph = this->property("listOfGraph").value<GraphParamList>();
}

QDataStream &operator<<(QDataStream &stream, const Chart &obj)
{
    for (int i=0; i<obj.metaObject()->propertyCount(); ++i)
        if (obj.metaObject()->property(i).isStored(&obj))
            stream << obj.metaObject()->property(i).read(&obj);

    for (auto &byteArray : obj.dynamicPropertyNames())
        stream << obj.property(byteArray);

    return stream;
}

QDataStream &operator>>(QDataStream &stream, Chart &obj)
{
    QVariant var;
    for (int i=0; i<obj.metaObject()->propertyCount(); ++i) {
        if (obj.metaObject()->property(i).isStored(&obj)) {
            stream >> var;
            if (!var.isNull())
                obj.metaObject()->property(i).write(&obj, var);
        }
    }
    obj.setProperties();

    for (auto &byteArray : obj.dynamicPropertyNames()) {
        stream >> var;
        obj.setProperty(byteArray, QVariant(var));
    }
    obj.setParamFromProperties();

    return stream;
}
