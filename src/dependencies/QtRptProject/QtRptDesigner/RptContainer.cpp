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

#include "RptContainer.h"

RptContainer::RptContainer(QWidget *parent, QPoint p, QWidget *cWidget) : XYZContainer(parent,p,cWidget) {
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    m_printing = "1";
    m_groupName = "";
    backgroundColor = Qt::white;
    borderWidth = 1;
    borderColor = Qt::black;
    setHasOverlay(true);
    menu = new QMenu(this);
}

void RptContainer::allowEditing(bool value) {
    this->m_isDesigning = value;
}

void RptContainer::setType(FieldType value) {
    m_type = value;
}

void RptContainer::loadParamFromXML(QDomElement e) {
    m_type = QtRPT::getFieldType(e);;
    this->setVisible(true);
    this->setSelected(false);
    scale = 1;
    setType(m_type);

    QRect r = this->geometry();
    r.setHeight(e.attribute("height").toInt());
    r.setWidth(e.attribute("width").toInt());
    this->setGeometry(r);
    this->setBaseSize(e.attribute("width").toInt(),e.attribute("height").toInt());
    this->setObjectName(e.attribute("name"));
    this->m_printing = e.attribute("printing","1");
    this->m_groupName = e.attribute("groupName","");
    this->borderWidth = e.attribute("borderWidth","1px").replace("px","").toInt();
    this->setSheetValue(FrameWidth,e.attribute("borderWidth","1px"));
    this->setSheetValue(FrameStyle,e.attribute("borderStyle","solid"));
}

QDomElement RptContainer::saveParamToXML(QDomDocument *xmlDoc) {
    QDomElement elem = xmlDoc->createElement(this->metaObject()->className());

    elem.setAttribute("type",QtRPT::getFieldTypeName(m_type));
    elem.setAttribute("printing",this->m_printing);
    elem.setAttribute("groupName",this->m_groupName);
    elem.setAttribute("name",this->objectName());
    elem.setAttribute("borderWidth",QString::number(getBorderWidth())+"px");

    elem.setAttribute("top",qRound(this->geometry().y()/scale));
    elem.setAttribute("left",qRound(this->geometry().x()/scale));
    elem.setAttribute("width",this->baseSize().width());
    elem.setAttribute("height",this->baseSize().height());
    return elem;
}

void RptContainer::setMenu(QMenu *menu_) {
    Q_UNUSED(menu_);
}

//Check, is there any side
bool RptContainer::borderIsCheck(Command command) {
    QString stl = this->styleSheet();
    int start=0; int end;
    if (command == FrameTop)
        start = stl.indexOf(";border-top-color:",0,Qt::CaseInsensitive);
    if (command == FrameLeft)
        start = stl.indexOf(";border-left-color:",0,Qt::CaseInsensitive);
    if (command == FrameRight)
        start = stl.indexOf(";border-right-color:",0,Qt::CaseInsensitive);
    if (command == FrameBottom)
        start = stl.indexOf(";border-bottom-color:",0,Qt::CaseInsensitive);

    end = stl.indexOf(";",start+1,Qt::CaseInsensitive);
    QString tmp = stl.mid(start+1,end-start-1);
    if (!tmp.contains("#ffffff") && !tmp.contains("rgba(255,255,255,0)"))  //If not white
        return true;
    else return false;
}

QString RptContainer::getBorderStyleStr() {
    int start = this->styleSheet().indexOf(";border-style:",0,Qt::CaseInsensitive);
    int end = this->styleSheet().indexOf(";",start+1,Qt::CaseInsensitive);
    QString tmp = this->styleSheet().mid(start+1,end-start-1);
    start = tmp.indexOf(":",0,Qt::CaseInsensitive);
    tmp = tmp.mid(start+1);
    return tmp;
}

void RptContainer::setBorderWidth(int value) {
    borderWidth = value;
}

int RptContainer::getBorderWidth() {
    return borderWidth;
}

void RptContainer::edit() {

}

void RptContainer::setBorder(Command command, QVariant values, bool yesFrame) {
    QColor color = values.value<QColor>();    
    if (color.isValid()) {
        setSheetValue(BorderColor,colorToString(color));
    } else {
        color = getColorValue(BorderColor);
    }

    borderColor = color;
    QString strColor = colorToString(color);
    QString stl = this->styleSheet();

    switch(command) {
        case None: {
            if (getColorValue(FrameTop) != "rgba(255,255,255,0)")
                setSheetValue(FrameTop,strColor);
            if (getColorValue(FrameBottom) != "rgba(255,255,255,0)")
                setSheetValue(FrameBottom,strColor);
            if (getColorValue(FrameLeft) != "rgba(255,255,255,0)")
                setSheetValue(FrameLeft,strColor);
            if (getColorValue(FrameRight) != "rgba(255,255,255,0)")
                setSheetValue(FrameRight,strColor);
            break;
        }
        case FrameNo: {
            setSheetValue(FrameTop,"rgba(255,255,255,0)");
            setSheetValue(FrameLeft,"rgba(255,255,255,0)");
            setSheetValue(FrameRight,"rgba(255,255,255,0)");
            setSheetValue(FrameBottom,"rgba(255,255,255,0)");
            break;
        }
        case FrameAll: {
            setSheetValue(FrameTop,strColor);
            setSheetValue(FrameLeft,strColor);
            setSheetValue(FrameRight,strColor);
            setSheetValue(FrameBottom,strColor);
            break;
        }
        case FrameTop:
        case FrameBottom:
        case FrameRight:
        case FrameLeft: {
            if (!yesFrame) strColor = "rgba(255,255,255,0)";
            setSheetValue(command,strColor);
            break;
        }
        case FrameStyle: {
            BorderStyle borderStyle = (BorderStyle)values.toInt();
            int start = stl.indexOf(";border-style:",0,Qt::CaseInsensitive);
            int end = stl.indexOf(";",start+1,Qt::CaseInsensitive);

            switch(borderStyle) {
                case Solid: {
                    stl.replace(start,end-start,";border-style:solid");
                    break;
                }
                case Dashed: {
                    stl.replace(start,end-start,";border-style:dashed");
                    break;
                }
                case Dotted: {
                    stl.replace(start,end-start,";border-style:dotted");
                    break;
                }
                case Dot_dash: {
                    stl.replace(start,end-start,";border-style:dot-dash");
                    break;
                }
                case Dot_dot_dash: {
                    stl.replace(start,end-start,";border-style:dot-dot-dash");
                    break;
                }
                case Double: {
                    stl.replace(start,end-start,";border-style:double");
                    break;
                }
                default: return;
            }
            setStyleSheet(stl);
            break;
        }
        case FrameWidth: {
            int start = stl.indexOf(";border-width:",0,Qt::CaseInsensitive);
            int end = stl.indexOf(";",start+1,Qt::CaseInsensitive);
            stl.replace(start,end-start,";border-width:"+values.toString()+"px");
            setStyleSheet(stl);
            borderWidth = values.toInt();
            break;
        }
        default: return;
    }
}

//Get color by command
QString RptContainer::getColorValueStr(Command param) {
    QString str1 = this->styleSheet();
    int start; int end;
    switch(param) {
        case FontColor: {
            start = str1.indexOf(";color:",0,Qt::CaseInsensitive);
            end = str1.indexOf(";",start+1,Qt::CaseInsensitive);
            break;
        }
        case BackgroundColor: {
            start = str1.indexOf(";background-color:",0,Qt::CaseInsensitive);
            end = str1.indexOf(";",start+1,Qt::CaseInsensitive);
            break;
        }
        case BorderColor: {
            start = str1.indexOf(";border-color:",0,Qt::CaseInsensitive);
            end = str1.indexOf(";",start+1,Qt::CaseInsensitive);
            break;
        }
        case FrameTop: {
            start = str1.indexOf(";border-top-color:",0,Qt::CaseInsensitive);
            end = str1.indexOf(";",start+1,Qt::CaseInsensitive);
            break;
        }
        case FrameBottom: {
            start = str1.indexOf(";border-bottom-color:",0,Qt::CaseInsensitive);
            end = str1.indexOf(";",start+1,Qt::CaseInsensitive);
            break;
        }
        case FrameLeft: {
            start = str1.indexOf(";border-left-color:",0,Qt::CaseInsensitive);
            end = str1.indexOf(";",start+1,Qt::CaseInsensitive);
            break;
        }
        case FrameRight: {
            start = str1.indexOf(";border-right-color:",0,Qt::CaseInsensitive);
            end = str1.indexOf(";",start+1,Qt::CaseInsensitive);
            break;
        }
    default: return "";
    }
    QString tmp = str1.mid(start+1,end-start-1);
    start = tmp.indexOf("rgba",0,Qt::CaseInsensitive);
    tmp = tmp.mid(start);
    if (start == -1) tmp = "";
    if (tmp == "#ffffff") tmp = "rgba(255,255,255,0)";
    return tmp;
}

void RptContainer::setColorValue(Command param, QColor value) {
    switch(param) {
        case FontColor: {
            fontColor = value;
            break;
        }
        case BackgroundColor: {
            backgroundColor = value;
            break;
        }
        case BorderColor:
        case FrameTop:
        case FrameBottom:
        case FrameLeft:
        case FrameRight: {
            borderColor = value;
            break;
        }
        default: return;
    }
}

QColor RptContainer::getColorValue(Command param) {
    switch(param) {
        case FontColor: {
            return fontColor;
            break;
        }
        case BackgroundColor: {
            return backgroundColor;
            break;
        }
        case BorderColor:
        case FrameTop:
        case FrameBottom:
        case FrameLeft:
        case FrameRight: {
            return borderColor;
            break;
        }
        default: return QColor();
    }
}

//Setup the style according the param
void RptContainer::setSheetValue(Command param, QString value) {
    QString str = this->styleSheet();
    int start; int end;
    if (value == "#ffffff") value = "rgba(255,255,255,0)";
    switch(param) {
        case FontColor: {
            start = str.indexOf(";color:",0,Qt::CaseInsensitive);
            end = str.indexOf(";",start+1,Qt::CaseInsensitive);
            str.replace(start,end-start,";color:"+value);
            fontColor = colorFromString(value);
            break;
        }
        case BackgroundColor: {
            start = str.indexOf(";background-color:",0,Qt::CaseInsensitive);
            end = str.indexOf(";",start+1,Qt::CaseInsensitive);
            str.replace(start,end-start,";background-color:"+value);
            backgroundColor = colorFromString(value);
            break;
        }
        case BorderColor: {
            start = str.indexOf(";border-color:",0,Qt::CaseInsensitive);
            end = str.indexOf(";",start+1,Qt::CaseInsensitive);
            str.replace(start,end-start,";border-color:"+value);
            borderColor = colorFromString(value);

            QString tmpColor = getColorValueStr(FrameTop);
            if (tmpColor != "rgba(255,255,255,0)") {
                start = str.indexOf(";border-top-color:",0,Qt::CaseInsensitive);
                end = str.indexOf(";",start+1,Qt::CaseInsensitive);
                str.replace(start,end-start,";border-top-color:"+value);
            }
            tmpColor = getColorValueStr(FrameBottom);
            if (tmpColor != "rgba(255,255,255,0)") {
                start = str.indexOf(";border-bottom-color:",0,Qt::CaseInsensitive);
                end = str.indexOf(";",start+1,Qt::CaseInsensitive);
                str.replace(start,end-start,";border-bottom-color:"+value);
            }
            tmpColor = getColorValueStr(FrameLeft);
            if (tmpColor != "rgba(255,255,255,0)") {
                start = str.indexOf(";border-left-color:",0,Qt::CaseInsensitive);
                end = str.indexOf(";",start+1,Qt::CaseInsensitive);
                str.replace(start,end-start,";border-left-color:"+value);
            }
            tmpColor = getColorValueStr(FrameRight);
            if (tmpColor != "rgba(255,255,255,0)") {
                start = str.indexOf(";border-right-color:",0,Qt::CaseInsensitive);
                end = str.indexOf(";",start+1,Qt::CaseInsensitive);
                str.replace(start,end-start,";border-right-color:"+value);
            }
            break;
        }
        case FrameTop: {
            start = str.indexOf(";border-top-color:",0,Qt::CaseInsensitive);
            end = str.indexOf(";",start+1,Qt::CaseInsensitive);
            str.replace(start,end-start,";border-top-color:"+value);
            break;
        }
        case FrameBottom: {
            start = str.indexOf(";border-bottom-color:",0,Qt::CaseInsensitive);
            end = str.indexOf(";",start+1,Qt::CaseInsensitive);
            str.replace(start,end-start,";border-bottom-color:"+value);
            break;
        }
        case FrameLeft: {
            start = str.indexOf(";border-left-color:",0,Qt::CaseInsensitive);
            end = str.indexOf(";",start+1,Qt::CaseInsensitive);
            str.replace(start,end-start,";border-left-color:"+value);
            break;
        }
        case FrameRight: {
            start = str.indexOf(";border-right-color:",0,Qt::CaseInsensitive);
            end = str.indexOf(";",start+1,Qt::CaseInsensitive);
            str.replace(start,end-start,";border-right-color:"+value);
            break;
        }
        case FrameWidth: {
            start = str.indexOf(";border-width:",0,Qt::CaseInsensitive);
            end = str.indexOf(";",start+1,Qt::CaseInsensitive);
            str.replace(start,end-start,";border-width:"+value);
            borderWidth = value.replace("px","").toInt();
            break;
        }
        case FrameStyle: {
            start = str.indexOf(";border-style:",0,Qt::CaseInsensitive);
            end = str.indexOf(";",start+1,Qt::CaseInsensitive);
            str.replace(start,end-start,";border-style:"+value);
            break;
        }
        default: return;
    }
    setStyleSheet(str);
}

RptContainer::~RptContainer() {
    emit delCont(this->itemInTree);
}

void RptContainer::keyPressEvent(QKeyEvent *e) {
    XYZContainer::keyPressEvent(e);
    if (e->key() == Qt::Key_Delete) {
        this->RptContainer::~RptContainer();
    }
}

