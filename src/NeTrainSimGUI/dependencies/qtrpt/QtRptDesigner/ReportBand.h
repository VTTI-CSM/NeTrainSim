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

#ifndef REPORTBAND_H
#define REPORTBAND_H

#include <QTreeWidgetItem>
#include "GraphicsLine.h"
#include "GraphicsBox.h"

class ReportBand : public GraphicsBox
{
    
public:
    static const int titleHeight = 16;

    explicit ReportBand(BandType type, int num);
    int type() const Q_DECL_OVERRIDE { return ItemType::GBand; }
    void setMenu(QMenu *menu);
    qreal scale;
    void setHeight(qreal value);
    QString getGroupingField();
    void setGroupingField(QString value);
    int getStartNewNumertaion();
    void setStartNewNumeration(bool value);
    bool getShowInGroup();
    void setShowInGroup(bool value);
    bool getStartNewPage();
    void setStartNewPage(bool value);
    int getGroupLevel();
    void setGroupLevel(int value);
    void loadParamFromXML(QDomElement e) override;
    QDomElement saveParamToXML(QSharedPointer<QDomDocument> xmlDoc) override;
    QString getDSName();
    void setDSName(QString value);
    BandType bandType;
    int bandNo;

private:
    bool m_infocus;
    QString m_groupingField;
    QString m_dsName;
    bool m_startNewNumeration;
    bool m_showInGroup;
    bool m_startNewPage;
    int m_groupLevel;


protected:
    QPoint position;
    QPointF mousePos;

};

#endif // REPORTBAND_H
