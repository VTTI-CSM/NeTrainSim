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

#ifndef REPSCROLLAREA_H
#define REPSCROLLAREA_H

#include <QScrollArea>
#include "PageSettingDlg.h"
#include "ReportBand.h"
#include "GraphicsScene.h"

namespace Ui {
    class RepScrollArea;
}

class RepScrollArea : public QScrollArea
{
    Q_OBJECT
    
public:
    explicit RepScrollArea(QTreeWidgetItem* rootItem, QWidget *parent = nullptr);
    QWidget* repWidget;
    ~RepScrollArea();
    double setPaperSize(qreal scale);
    ReportBand* m_addBand(BandType type, QMenu* bandMenu, int m_height=0, QString objName="", int bandNo=1);
    void newFieldTreeItem(QGraphicsItem* item);
    PageSetting pageSetting;
    void correctBandGeom(ReportBand *rep = nullptr);
    bool allowField();
    void clearReport();

    qreal getScale();
    GraphicsScene* scene;
    void setScale(const QString &scale);
    QList<ReportBand*> getReportBands();

protected:
    bool eventFilter(QObject *obj, QEvent *e);

private:
    Ui::RepScrollArea *ui;
    void paintHorRuler();
    void paintVerRuler();
    QWidget* m_mainWindow;
    double m_koef;
    qreal m_scale;
    QTreeWidgetItem* m_rootItem;
    void getKoef();
    void assignBandParam(BandType bandType, int &bandNo, int &lvl, QString &objName);

public slots:
    void showGrid(bool value);

private slots:
    void vScrolling(int value);

};

#endif // REPSCROLLAREA_H
