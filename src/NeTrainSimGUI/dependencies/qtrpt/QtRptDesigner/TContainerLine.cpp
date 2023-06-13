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


#include "TContainerLine.h"
#include "ReportBand.h"

TContainerLine::TContainerLine(QWidget *parent, QPoint p, QWidget *cWidget) : RptContainer(parent,p,cWidget) {
    QString stl = "TContainerLine#lbl {;"
                  "border-width:1px;"
                  "border-style:solid;"
                  "border-color:rgba(0,0,0,255);"
                  "border-top-color:rgba(0,0,0,255);"
                  "border-left-color:rgba(0,0,0,255);"
                  "border-right-color:rgba(0,0,0,255);"
                  "border-bottom-color:rgba(0,0,0,255);"
                  "color:rgba(0,0,0,255);"
                  "background-color:rgba(255,255,255,0);"
            "}";
    cs = 0;
    ce = 0;
    m_arrowStart = false;
    m_arrowEnd = true;
    this->setStyleSheet(stl);
    this->resize(10,10);
    this->setBaseSize(width(),height());
    this->allowResize(false);
    this->allowDrawSelection(false);
    this->setAutoFillBackground(true);
    this->move(-50,-50);

    line.setP1( QPoint() );
    line.setP2( QPoint() );

    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, Qt::blue);

    cs = new XYZContainer(parent,QPoint(line.p1().x(), line.p2().y()));
    cs->setObjectName("CS");
    cs->resize(6,6);
    cs->allowResize(false);
    cs->allowDrawSelection(false);
    cs->setVisible(false);
    cs->setAutoFillBackground(true);
    cs->setPalette(Pal);

    ce = new XYZContainer(parent,QPoint(line.p2().x(), line.p2().y()));
    ce->setObjectName("CE");
    ce->resize(6,6);
    ce->allowResize(false);
    ce->allowDrawSelection(false);
    ce->setVisible(false);
    ce->setAutoFillBackground(true);
    ce->setPalette(Pal);

    QObject::connect(cs, SIGNAL(newGeometry(QRect, QRect)), this, SIGNAL(newGeometry(QRect, QRect)));
    QObject::connect(ce, SIGNAL(newGeometry(QRect, QRect)), this, SIGNAL(newGeometry(QRect, QRect)));
    QObject::connect(ce, SIGNAL(newGeometry(QRect, QRect)), this, SLOT(lineChanged(QRect, QRect)));
    QObject::connect(cs, SIGNAL(newGeometry(QRect, QRect)), this, SLOT(lineChanged(QRect, QRect)));
    //QObject::connect(cs, SIGNAL(geomChanged(QRect, QRect)), this, SIGNAL(geomChanged(QRect, QRect)));
    //QObject::connect(ce, SIGNAL(geomChanged(QRect, QRect)), this, SIGNAL(geomChanged(QRect, QRect)));
    QObject::connect(cs, SIGNAL(geomChanged(QRect, QRect)), this, SLOT(geomContChanged(QRect, QRect)));
    QObject::connect(ce, SIGNAL(geomChanged(QRect, QRect)), this, SLOT(geomContChanged(QRect, QRect)));

    QObject::connect(cs, SIGNAL(destroyed()), this, SLOT(delItemInTree()));
    QObject::connect(ce, SIGNAL(destroyed()), this, SLOT(delItemInTree()));

    QObject::connect(this, SIGNAL(delCont(QTreeWidgetItem *)), this, SLOT(delItemInTree()));

    QObject::connect(cs, SIGNAL(inFocus(bool)), this, SLOT(m_inFocus(bool)));
    QObject::connect(ce, SIGNAL(inFocus(bool)), this, SLOT(m_inFocus(bool)));

    //QObject::connect(this, SIGNAL(inFocus(bool)), cs, SIGNAL(inFocus(bool)));
    //QObject::connect(this, SIGNAL(inFocus(bool)), ce, SIGNAL(inFocus(bool)));

    this->show();
}

void TContainerLine::geomContChanged(QRect oldRect, QRect newRect) {
    if (sender() == cs) {
        m_oldP1 = oldRect;
        m_oldP2 = ce->geometry();
    }
    if (sender() == ce) {
        m_oldP1 = cs->geometry();
        m_oldP2 = oldRect;
    }
    emit geomChanged(oldRect,newRect);
}

void TContainerLine::delItemInTree() {
    delete this;
}

void TContainerLine::setArrow(Command command, QVariant value) {
    if (command == ArrowStart)
        m_arrowStart = value.toBool();
    if (command ==ArrowEnd)
        m_arrowEnd = value.toBool();
    this->parentWidget()->repaint();
}

bool TContainerLine::getArrow(Command command) {
    bool result = false;
    if (command == ArrowStart)
        result = m_arrowStart;
    if (command ==ArrowEnd)
        result = m_arrowEnd;
    return result;
}

void TContainerLine::focusInEvent(QFocusEvent *e) {
    XYZContainer::focusInEvent(e);
    setLine(true);
}

void TContainerLine::focusOutEvent(QFocusEvent *e) {    
    XYZContainer::focusInEvent(e);
    if (cs != 0 && ce != 0)
        setSelectedLine(QPoint(0,0));
}

void TContainerLine::m_inFocus(bool value) {
    setLine(value);
    emit inFocus(value);
}

void TContainerLine::setLine(bool value) {
    QPalette pal(Qt::white);
    bool selected = false;
    if (value) {
        pal.setColor(QPalette::Background, Qt::blue);
        selected = true;
    }

    ce->setPalette(pal);
    cs->setPalette(pal);
    ce->setVisible(selected);
    cs->setVisible(selected);
}

void TContainerLine::setSelectedLine(QPoint point) {
    QList<TContainerLine *> contLineList = this->parentWidget()->findChildren<TContainerLine *>();
    foreach (TContainerLine *contLine, contLineList) {
        bool selected = false;
        QPointF intersectPnt;
        QLineF line(point.x()-5, point.y()-5, point.x()+5, point.y()+5);
        if (contLine->line.intersect(line, &intersectPnt) == QLineF::BoundedIntersection) {
            if (!contLine->hasFocus()) {
                contLine->setFocus();
            }
            selected = true;
        }
        if (contLine->ce->hasFocus() && this->ce->rect().contains(point)) {
            selected = true;
        }
        if (contLine->cs->hasFocus() && this->cs->rect().contains(point)) {
            selected = true;
        }

        contLine->setLine(selected);
    }
}

void TContainerLine::lineChanged(QRect, QRect) {
    if (cs != 0 && !cs->pos().isNull())
        line.setP1( cs->pos()+QPoint(3,3) );
    if (ce != 0 && !ce->pos().isNull())
        line.setP2( ce->pos()+QPoint(3,3) );
}

void TContainerLine::movePoint(XYZContainer *cont, QRect rect) {
    cont->move(rect.x(),rect.y());
    lineChanged(QRect(),QRect());
    this->parentWidget()->repaint();
}

void TContainerLine::setParent(QWidget *parent) {
    RptContainer::setParent(parent);
    ce->setParent(parent);
    cs->setParent(parent);
}

void TContainerLine::setObjectName(const QString &name) {
    const QString old_name = this->objectName();
    QObject::setObjectName(name);
    QString str = this->styleSheet().replace("lbl",name).replace(old_name,name);
    this->setStyleSheet(str);
}

void TContainerLine::loadParamFromXML(QDomElement e) {
    RptContainer::loadParamFromXML(e);

    //this->setSheetValue(BackgroundColor,e.attribute("backgroundColor","rgba(255,255,255,0)"));
    this->setSheetValue(BorderColor,e.attribute("borderColor","rgba(0,0,0,255)"));

    this->line.setP1(QPointF(e.attribute("lineStartX","0").toDouble(), e.attribute("lineStartY","0" ).toDouble()));
    this->line.setP2(QPointF(e.attribute("lineEndX","0").toDouble(), e.attribute("lineEndY","0" ).toDouble()));
    this->m_arrowStart = e.attribute("arrowStart","0").toInt();
    this->m_arrowEnd = e.attribute("arrowEnd","0").toInt();
    this->cs->move(this->line.toLine().p1()-QPoint(3,3));
    this->ce->move(this->line.toLine().p2()-QPoint(3,3));
}

QDomElement TContainerLine::saveParamToXML(QDomDocument *xmlDoc) {
    QDomElement elem = RptContainer::saveParamToXML(xmlDoc);

    QString borderColor = colorToString(getColorValue(BorderColor));
    elem.setAttribute("borderColor",borderColor);
    elem.setAttribute("borderStyle",getBorderStyleStr());
    elem.setAttribute("lineStartX",this->line.p1().x());
    elem.setAttribute("lineStartY",this->line.p1().y());
    elem.setAttribute("lineEndX",this->line.p2().x());
    elem.setAttribute("lineEndY",this->line.p2().y());
    elem.setAttribute("arrowStart",this->m_arrowStart);
    elem.setAttribute("arrowEnd",this->m_arrowEnd);

    return elem;
}

void TContainerLine::setMenu(QMenu *menu_) {
    QIcon icon;
    QAction *actContDel = new QAction(tr("Delete"),this);
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/delete.png")), QIcon::Normal, QIcon::On);
    actContDel->setObjectName("actContDel");
    actContDel->setIcon(icon);
    QObject::connect(actContDel, SIGNAL(triggered()), this, SIGNAL(deleteByUser()));
    QObject::connect(actContDel, SIGNAL(triggered()), this, SLOT(deleteLater()));

    menu->clear();
    menu->insertActions(0,menu_->actions());
    menu->addAction(actContDel);
}

TContainerLine *TContainerLine::clone() {
    TContainerLine *newContField  = new TContainerLine(this->parentWidget(),QPoint(0,0),0);
    newContField->setType(this->getType());
    newContField->setStyleSheet(this->styleSheet());
    newContField->setGeometry(this->geometry());
    newContField->setBaseSize(this->baseSize());
    newContField->setVisible(true);
    newContField->move(-10,-10);
    newContField->line.setP1( this->line.p1()+QPointF(5,5));
    newContField->line.setP2( this->line.p2()+QPointF(5,5));
    newContField->cs->move( this->cs->pos()+QPoint(5,5) );
    newContField->ce->move( this->ce->pos()+QPoint(5,5) );
    newContField->setArrow(ArrowStart, this->getArrow(ArrowStart));
    newContField->setArrow(ArrowEnd, this->getArrow(ArrowEnd));
    newContField->setColorValue(BorderColor,this->getColorValue(BorderColor));
    newContField->setBorderWidth(this->getBorderWidth());
    return newContField;
}

qreal TContainerLine::getLength() {
    return line.length();
}

void TContainerLine::drawArrow(QPainter *painter) {
    // Draw the arrows
    static const double Pi = 3.14159265358979323846264338327950288419717;
    static double TwoPi = 2.0 * Pi;
    double angle = ::acos(line.dx() / line.length());
    if (line.dy() >= 0)
         angle = TwoPi - angle;

     QPointF sourcePoint = line.p1();
     QPointF destPoint = line.p2();
     int arrowSize= 10;

     painter->setBrush(getColorValue(BorderColor));

     if (m_arrowStart) {
         QPointF sourceArrowP1 = sourcePoint + QPointF(sin(angle + Pi / 3) * arrowSize,
                                                       cos(angle + Pi / 3) * arrowSize);
         QPointF sourceArrowP2 = sourcePoint + QPointF(sin(angle + Pi - Pi / 3) * arrowSize,
                                                       cos(angle + Pi - Pi / 3) * arrowSize);
         painter->drawPolygon(QPolygonF() << line.p1() << sourceArrowP1 << sourceArrowP2);
     }
     if (m_arrowEnd) {
         QPointF destArrowP1 = destPoint + QPointF(sin(angle - Pi / 3) * arrowSize,
                                                   cos(angle - Pi / 3) * arrowSize);
         QPointF destArrowP2 = destPoint + QPointF(sin(angle - Pi + Pi / 3) * arrowSize,
                                                   cos(angle - Pi + Pi / 3) * arrowSize);
         painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);
     }
}

TContainerLine::~TContainerLine() {
    if (cs != 0) {
        cs->deleteLater();
        cs = 0;
    }
    if (ce != 0) {
        ce->deleteLater();
        ce = 0;
    }
}

void TContainerLine::setProperties() {
    this->setProperty("FieldType",m_type);
}

//Restore fields from properties
void TContainerLine::setParamFromProperties() {
    m_type = (FieldType)this->property("FieldType").toInt();
}

QDataStream &operator<<(QDataStream &stream, const TContainerLine &obj) {
    for(int i=0; i<obj.metaObject()->propertyCount(); ++i) {
        if(obj.metaObject()->property(i).isStored(&obj)) {
            stream << obj.metaObject()->property(i).read(&obj);
        }
    }
    QList<QByteArray> list = obj.dynamicPropertyNames();
    for (int i=0; i<list.size(); i++) {
        stream << obj.property(list.at(i));
    }
    stream << *obj.cs;
    stream << *obj.ce;

    return stream;
}

QDataStream &operator>>(QDataStream &stream, TContainerLine &obj) {
    QVariant var;
    for(int i=0; i<obj.metaObject()->propertyCount(); ++i) {
        if(obj.metaObject()->property(i).isStored(&obj)) {
            stream >> var;
            if (!var.isNull())
                obj.metaObject()->property(i).write(&obj, var);
        }
    }
    obj.setProperties();
    QList<QByteArray> list = obj.dynamicPropertyNames();
    for (int i=0; i<list.size(); i++) {
        stream >> var;
        obj.setProperty(list.at(i),QVariant(var));
    }
    obj.setParamFromProperties();

    stream >> *obj.cs;
    stream >> *obj.ce;
    return stream;
}
