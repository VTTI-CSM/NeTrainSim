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

#ifndef CHART_H
#define CHART_H

#include <QPainter>
#include <QWidget>
#include <QPaintEvent>
#include <QStyleOption>
#include <QDomElement>
#include <QDebug>
#include <CommonClasses.h>

enum ChartParam {
    DrawGrid = 0,
    ShowCaption = 1,
    ShowLegend = 2,
    ShowGraphCaption = 3,
    ShowPercent = 4,
    Caption = 5,
    AutoFillData = 6
};

struct GraphParam
{
    QColor color;
    float valuePercent;
    float valueReal;
    QString valueString;
    QString formula;
    QString caption;
};

typedef QList<GraphParam> GraphParamList;

Q_DECLARE_METATYPE(GraphParam)
Q_DECLARE_METATYPE(GraphParamList)
Q_DECLARE_METATYPE(QDomElement)

class Chart : public QWidget {
    Q_OBJECT    
public:
    explicit Chart(QWidget *parent = 0);
    void setParams(bool drawGrid, bool drawHistory, bool drawCaption, bool drawGraphCaption, bool showPercent, QString caption, bool autoFillData);
    void setData(GraphParam param, float value100Percent=0);
    QVariant getParam(ChartParam param);
    GraphParamList getGraphParamList();
    void paintChart(QPainter *painter);
    void setKoef(float koefRes_w, float koefRes_h, float left, float top);
    void clearData();
    void loadXML(QDomElement e);
    void setProperties();
    friend QDataStream &operator<<(QDataStream &stream, const Chart &obj);
    friend QDataStream &operator>>(QDataStream &stream, Chart &obj);

protected:
    void paintEvent(QPaintEvent * event);
    void setParamFromProperties();

private:
    QList<GraphParam> listOfGraph;
    QString m_caption;
    bool m_drawGrid;
    bool m_drawHistory;
    bool m_drawCaption;
    bool m_drawGraphCaption;
    bool m_showPercent;
    bool m_autoFillData;
    int m_rightSpaceWidth;
    int m_bottomSpaceHeight;
    int m_captionHeight;
    float m_correctValue;
    float m_koefRes_h;
    float m_koefRes_w;
    float m_left;
    float m_top;

signals:

public slots:

};

using SPtrChart = QSharedPointer<Chart>;

#endif // CHART_H
