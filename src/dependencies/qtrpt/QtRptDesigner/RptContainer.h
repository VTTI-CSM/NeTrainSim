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

#ifndef RPTCONTAINER_H
#define RPTCONTAINER_H

#include <QLabel>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMenu>
#include <QTreeWidgetItem>
#include <QDomElement>
#include <XYZ_Container.h>
#include <CommonClasses.h>
#include <qtrptnamespace.h>
#include <qtrpt.h>

class RptContainer : public XYZContainer {
    Q_OBJECT    
    Q_PROPERTY(QString printing READ getPrinting WRITE setPrinting)
    Q_PROPERTY(QColor bgColor READ getBgColor WRITE setBgColor)
    Q_PROPERTY(QColor BrdColor READ getBorderColor WRITE setBorderColor)
    Q_ENUMS(FieldType)
public:
    RptContainer(QWidget *parent, QPoint p, QWidget *cWidget = 0);
    ~RptContainer();
    QTreeWidgetItem *itemInTree;
    QString getColorValueStr(Command param);
    QColor getColorValue(Command param);
    bool borderIsCheck(Command command);
    void setBorder(Command command, QVariant values, bool yesFrame = false);
    int getBorderWidth();
    QString getBorderStyleStr();
    void setSheetValue(Command param, QString value);
    void allowEditing(bool value);
    virtual void setType(FieldType value);
    FieldType getType() {return m_type;}
    virtual void setMenu(QMenu *menu_);
    virtual void loadParamFromXML(QDomElement e);
    virtual QDomElement saveParamToXML(QDomDocument *xmlDoc);
    QString getPrinting() {return m_printing;}
    void setPrinting(QString value) {m_printing = value;}
    QString getGroupName() {return m_groupName;}
    void setGroupName(QString value) {m_groupName = value;}
    virtual void setProperties(){}

    void *operator new(size_t size) {return ::operator new(size);}  //make a standart operator visible
    void *operator new(size_t size, void *ptr) { //overloaded operator
        Q_UNUSED(size)
        return ptr;
    }

protected:
    FieldType m_type;
    void setColorValue(Command param, QColor value);    
    void setBorderWidth(int value);
    void keyPressEvent(QKeyEvent *e);

private:
    QColor backgroundColor;
    QColor borderColor;
    QColor fontColor;
    int borderWidth;
    QString m_printing;
    QString m_groupName;

    void setBgColor(QColor value) {backgroundColor = value;}
    QColor getBgColor() {return backgroundColor;}
    void setBorderColor(QColor value) {borderColor = value;}
    QColor getBorderColor() {return borderColor;}

public slots:
    virtual void edit();

private slots:

signals:
    void delCont(QTreeWidgetItem *);
    void contChanged(bool);
};

QDataStream &operator<<(QDataStream &stream, const RptContainer &obj);
QDataStream &operator>>(QDataStream &stream, RptContainer &obj) ;

typedef QList<RptContainer *> RptContainerList;

#endif // RPTCONTAINER_H
