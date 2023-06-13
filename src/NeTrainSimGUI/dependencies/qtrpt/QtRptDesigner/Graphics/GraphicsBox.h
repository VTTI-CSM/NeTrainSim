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

#pragma once

#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include "CornerGrabber.h"
#include "GraphicsHelperClass.h"
#include <QTextDocument>
#include <QObject>
#include <QDomDocument>
#include <QGraphicsProxyWidget>
#include <QPushButton>
#include <Barcode.h>


using SPtrQChartView = QSharedPointer<QChartView>;
using SPtrQChart = QSharedPointer<QChart>;

class GraphicsBox : public QGraphicsItem, public GraphicsHelperClass
{
public:
    GraphicsBox();
    ~GraphicsBox();
    virtual void setPos(QPointF pos);
    virtual void setPos(qreal x, qreal y);
    QPointF getPos();
    int type() const Q_DECL_OVERRIDE { return ItemType::GBox; }
    void setSelected(bool selected);
    bool isSelected();
    void setWidth(qreal value);
    void setHeight(qreal value);
    qreal getWidth() {return m_width;}
    qreal getHeight() {return m_height;}
    GraphicsBox *clone();
    void loadParamFromXML(QDomElement e) override;
    QDomElement saveParamToXML(QSharedPointer<QDomDocument> xmlDoc) override;
    void setText(QString value);
    QString getText();
    void setFont(QFont font) {m_font = font;}
    QFont getFont() {return m_font;}
    Qt::Alignment getAlignment() {return m_alignment;}
    void setAlignment(Qt::Alignment value) {m_alignment = value;}
    void setBandPixmap(QPixmap pixmap) {m_bandPixmap = pixmap;}
    void setBorderVisible(bool value);
    bool borderIsVisible();
    bool getIgnoreAspectRatio() {return m_ignoreAspectRatio;}
    void setIgnoreAspectRatio(bool value) {m_ignoreAspectRatio = value;}
    bool getAutoHeight() {return m_autoHeight;}
    void setAutoHeight(bool value) {m_autoHeight = value;}
    bool getRenderingMode() {return m_RenderingMode;}
    void setRenderingMode(bool value) {m_RenderingMode = value;}
    void setTextDirection(bool value) {m_textDirection = value;}
    bool getTextDirection() {return m_textDirection;}
    void setTextWrap(bool value) {m_textWrap = value;}
    bool textWrap() {return m_textWrap;}
    void setTextRotate(int value) {m_textRotate = value;}
    int textRotate() {return m_textRotate;}
    QString getFormatString() {return m_formatString;}
    void setFormatString(QString value) {m_formatString = value;}
    QString getInputFormatString() {return m_inputFormatString;}
    void setInputFormatString(QString value) {
        m_inputFormatString = value;
    }

    void setImage(QPixmap pixmap);
    QPixmap getImage();
    QString getImgFormat();
    void setImgFromat(QString value);
    void setMenu(QMenu *menu);
    BarCode::BarcodeTypes getBarcodeType();
    void setBarcodeType(BarCode::BarcodeTypes value);
    BarCode::FrameTypes getBarcodeFrameType();
    void setBarcodeFrameType(BarCode::FrameTypes value);
    int getBarcodeHeight();
    void setBarcodeHeight(int value);
    void setFieldType(FieldType value);
    QString getHighlighting() {return m_highlighting;}
    void setHighlighting(QString value) {m_highlighting = value;}
    SPtrCrossTab getCrossTab();
    SPtrQChart getChart();
    SPtrBarCode getBarCode();

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    qreal   m_drawingWidth;
    qreal   m_drawingHeight;
    qreal   m_drawingOrigenX;
    qreal   m_drawingOrigenY;

    void createCorners();
    void setCornerPositions();
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) Q_DECL_OVERRIDE;

private:
    QRectF boundingRect() const Q_DECL_OVERRIDE; // must be re-implemented in this class to provide the diminsions of the box to the QGraphicsView
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) Q_DECL_OVERRIDE; // must be re-implemented here to pain the box on the paint-event
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE; // must be re-implemented to handle mouse hover enter events
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE; // must be re-implemented to handle mouse hover leave events

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;

    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QGraphicsSceneDragDropEvent *event);
    void mousePressEvent(QGraphicsSceneDragDropEvent *event);


    void adjustSize(int x, int y);

    qreal   m_width;
    qreal   m_height;

    QString m_text;
    bool m_borderIsVisible;
    int m_RenderingMode;

    QPixmap m_bandPixmap;
    QFont m_font;
    Qt::Alignment m_alignment;
    bool m_ignoreAspectRatio;
    bool m_autoHeight;
    bool m_textDirection;
    bool m_textWrap;
    int m_textRotate;
    int m_radius;
    SPtrBarCode m_barcode;
    SPtrQChartView m_chartView;
    SPtrQChart m_chart;
    SPtrCrossTab m_crossTab;

    QString m_formatString;
    QString m_inputFormatString;
    QString m_highlighting;
    QString m_imgFormat;
    QPixmap m_pixmap;

};
