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

#include <RptFieldObject.h>
#include <QObject>

using namespace QtRptName;

class QtRPT;
class RptPageObject;
class RptFieldObject;

class RptBandObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString groupingField MEMBER groupingField)
    Q_PROPERTY(bool startNewPage MEMBER startNewPage)
    Q_PROPERTY(bool showInGroup MEMBER showInGroup)
    Q_PROPERTY(bool startNewNumeration MEMBER startNewNumeration)
    Q_PROPERTY(int groupLevel MEMBER groupLevel)
    Q_PROPERTY(int realHeight MEMBER realHeight)
    Q_PROPERTY(int height MEMBER height)
    Q_PROPERTY(int bandNo MEMBER bandNo)
    Q_PROPERTY(QString dsName MEMBER dsName)
    Q_PROPERTY(bool sortDataInGroup MEMBER sortDataInGroup)
    Q_PROPERTY(bool groupHeaderEachlevel MEMBER groupHeaderEachlevel)

    friend class QtRPT;
    friend class RptPageObject;
public:
    RptBandObject(QObject *parent = nullptr);
    ~RptBandObject();
    QString name;
    QString groupingField;
    bool showInGroup;
    bool groupHeaderEachlevel;
    bool startNewPage;
    bool startNewNumeration;
    bool sortDataInGroup;
    int groupLevel;
    int realHeight;
    int height;
    int width;
    int left;
    int right;
    BandType type;
    int bandNo;
    QString dsName;
    void addField(RptFieldObject *field);
    QList<RptFieldObject*> fieldList;
    RptPageObject *parentReportPage;
    RptBandObject *clone();
    Q_INVOKABLE void setStartNewPage(bool value);

private:
	QtRPT *m_qtrpt;
    void setProperty(QtRPT *qtrpt, QDomElement docElem);

};

QDebug operator<<(QDebug dbg, const RptBandObject &obj);
