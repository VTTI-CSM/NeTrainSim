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
#include <qtrptnamespace.h>
#include <RptBandObject.h>
#include <RptFieldObject.h>
#include <RptSql.h>
#include <RptDsInline.h>

using namespace QtRptName;


class QtRPT;
class RptBandObject;
class RptFieldObject;

typedef QList<RptBandObject*> BandList;
Q_DECLARE_METATYPE(BandList)

class RptPageObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int pageNo MEMBER pageNo)
    Q_PROPERTY(int orientation MEMBER orientation)
    Q_PROPERTY(BandList bandList MEMBER bandList)
    Q_PROPERTY(bool border MEMBER border)

    friend class QtRPT;

public:
    RptPageObject(QtRPT *qtrpt = nullptr);
    ~RptPageObject();
    int pageNo;
    int orientation;
    int ph;
    int pw;
    int ml;
    int mr;
    int mt;
    int mb;
    bool border;
    int borderWidth;
    QColor borderColor;
    QString borderStyle;
    bool watermark;
    float watermarkOpacity;
    QPixmap watermarkPixmap;
    RptDsAbstract *rptDsInline;
    RptSql *rtpSql;
    RptSqlConnection sqlConnection;

    void addBand(RptBandObject *band);
    RptBandObject *getBand(BandType type, int No, int groupLevel = 0);
    Q_INVOKABLE RptBandObject *getBand(QString name);
    int bandsCountByType(BandType type);
    Q_INVOKABLE RptFieldObject *findFieldObjectByName(QString name);
    void initCrossTabProcessedRows();
    int crossTabParts();

    Q_INVOKABLE void setVisible(bool value);
    bool isVisible();

    quint16 totalPages();
    void setTotalPages(quint16 value);
    QList<RptFieldObject*> crossTabs();
    BandList bandList;
    int recordCount;
    RptPageObject *clone();

private:
	QtRPT *m_qtrpt;
    bool m_visible;
    quint16 m_totalPages;
    void setProperty(QtRPT *qtrpt, QDomElement docElem);

};

QDebug operator<<(QDebug dbg, const RptPageObject &obj);
