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

#include <QObject>
#include <QMenu>
#include <QGraphicsItem>
#include <CommonClasses.h>
#include <QTreeWidgetItem>
#include "qtrptnamespace.h"
#include "GraphicsNameSpace.h"
#include <qtrpt.h>
#include "CornerGrabber.h"

class GraphicsHelperClass : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString printing READ getPrinting WRITE setPrinting)
    Q_PROPERTY(int BrdWidth READ getBorderWidth WRITE setBorderWidth)
    Q_PROPERTY(QColor BrdColor READ getBorderColor WRITE setBorderColor)
    Q_PROPERTY(QColor BgColor READ getBackgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QString GrpName READ getGroupName WRITE setGroupName)
public:
    explicit GraphicsHelperClass(QObject *parent = nullptr);
    void setObjectName(const QString &name);
    bool getDrawingState() {return m_drawingState;}
    void setDrawingState(bool value) {m_drawingState = value;}
    QGraphicsItem *graphicsItem;
    QtRptName::FieldType getFieldType() {return m_type;}
    void setFieldType(QtRptName::FieldType value) {m_type = value;}
    QTreeWidgetItem *itemInTree;
    QString getPrinting() {return m_printing;}
    void setPrinting(QString value) {m_printing = value;}
    QColor getColorValue(Command param);
    void setColorValue(Command param, QColor value);
    void setBorderWidth(int value);
    int getBorderWidth();
    void setBorderColor(QColor value) {m_borderColor = value;}
    QColor getBorderColor() {return m_borderColor;}
    void setBackgroundColor(QColor value) {m_backgroundColor = value;}
    QColor getBackgroundColor() {return m_backgroundColor;}
    bool borderIsCheck(Command command);
    void setBorder(Command command, QVariant values, bool yesFrame = false);
    Qt::PenStyle borderStyle() {return m_borderStyle;}
    void setBorderStyle(Qt::PenStyle value) {m_borderStyle = value;}
    virtual void loadParamFromXML(QDomElement e);
    virtual QDomElement saveParamToXML(QSharedPointer<QDomDocument> xmlDoc);
    QString getGroupName() {return m_groupName;}
    void setGroupName(QString value) {m_groupName = value;}
    bool helperIsSelected();
    void helperSelect(bool value);

    friend QDataStream &operator<<(QDataStream &stream, const GraphicsHelperClass &obj);
    friend QDataStream &operator>>(QDataStream &stream, GraphicsHelperClass &obj);

protected:
    QColor m_outterborderColor; // the hover event handlers will toggle this between red and black
    QColor m_borderColor;
    QColor m_backgroundColor;
    QColor m_fontColor;
    QVector<SPtrCorner> m_corners;
    QMenu* m_menu;
    QtRptName::FieldType m_type;
    QString setPenStyle(Qt::PenStyle style);
    QPen m_outterborderPen; // the pen is used to paint the red/black border
    int m_XcornerGrabBuffer;
    int m_YcornerGrabBuffer;
    QPointF m_location;
    QPointF m_dragStart;
    void destroyCorners();

private:
    bool m_drawingState;
    QString m_printing;
    int borderWidth;
    Qt::PenStyle m_borderStyle;
    QColor m_borderLeftColor;
    QColor m_borderRightColor;
    QColor m_borderTopColor;
    QColor m_borderBottomColor;
    QString m_groupName;

signals:
    void itemRemoving();

public slots:
    void edit();
    void moveForward();
    void moveBack();

};
