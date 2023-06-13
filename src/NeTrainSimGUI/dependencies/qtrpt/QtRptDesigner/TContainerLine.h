/*
Name: QtRpt
Version: 1.5.5
Web-site: http://www.qtrpt.tk
Programmer: Aleksey Osipov
E-mail: aliks-os@ukr.net
Web-site: http://www.aliks-os.tk

Copyright 2012-2015 Aleksey Osipov

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


#ifndef TContainerLine_H
#define TContainerLine_H

#include <QWidget>
#include <XYZ_Container.h>
#include <QLabel>
#include "RptContainer.h"

class TContainerLine : public RptContainer
{
    Q_OBJECT
    Q_PROPERTY(QLineF Line READ getLine WRITE setLine)
    Q_PROPERTY(QRect OldP1 READ getOldP1 WRITE setOldP1)
    Q_PROPERTY(QRect OldP2 READ getOldP2 WRITE setOldP2)
    Q_PROPERTY(bool arrowStar READ getArrowStart WRITE setArrowStart)
    Q_PROPERTY(bool arrowEnd READ getArrowEnd WRITE setArrowEnd)
public:
    TContainerLine(QWidget *parent, QPoint p, QWidget *cWidget = 0);
    ~TContainerLine();
    QLineF line;
    XYZContainer *cs;
    XYZContainer *ce;
    qreal getLength();
    TContainerLine *clone();
    void setMenu(QMenu *menu_);
    void loadParamFromXML(QDomElement e);
    QDomElement saveParamToXML(QDomDocument *xmlDoc);
    void setParent(QWidget *parent);
    void drawArrow(QPainter *painter);
    void setSelectedLine(QPoint point);
    void setArrow(Command command, QVariant value);
    bool getArrow(Command command);
    void movePoint(XYZContainer *cont, QRect rect);
    QRect m_oldP1, m_oldP2;
    void setProperties();
    void setParamFromProperties();

    friend QDataStream &operator<<(QDataStream &stream, const TContainerLine &obj);
    friend QDataStream &operator>>(QDataStream &stream, TContainerLine &obj);

protected:
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);

private:
    bool m_arrowStart;
    bool m_arrowEnd;
    void setLine(bool value);
    void setObjectName(const QString &name);

    //Getter-Setter
    QLineF getLine() {return line;}
    void setLine(QLineF value) {line = value;}
    QRect getOldP1() {return m_oldP1;}
    void setOldP1(QRect value) {m_oldP1 = value;}
    QRect getOldP2() {return m_oldP2;}
    void setOldP2(QRect value) {m_oldP2 = value;}
    bool getArrowStart() {return m_arrowStart;}
    void setArrowStart(bool value) {m_arrowStart = value;}
    bool getArrowEnd() {return m_arrowEnd;}
    void setArrowEnd(bool value) {m_arrowEnd = value;}

signals:

public slots:

private slots:
    void lineChanged(QRect, QRect);
    void delItemInTree();
    void geomContChanged(QRect, QRect newRect);
    void m_inFocus(bool value);

};

#endif // TContainerLine_H
