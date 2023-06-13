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

#include <QDialog>
#include <QTextCharFormat>
#include <QTableWidgetItem>
#include "FldPropertyDlg.h"
#include "selectcolor.h"
#include "Barcode.h"

namespace Ui {
    class EditFldDlg;
}

class EditFldDlg : public QDialog
{
    Q_OBJECT

public:
    explicit EditFldDlg(QWidget *parent = nullptr);
    ~EditFldDlg();
    int showText(QGraphicsItem *gItem);
    int showTextRich(QGraphicsItem *gItem);
    int showImage(QGraphicsItem *gItem);
    int showDiagram(QGraphicsItem *gItem);
    int showBarcode(QGraphicsItem *gItem);
    int showCrosstab(QGraphicsItem *gItem);

protected:

private:
    Ui::EditFldDlg *ui;
    bool boolImage;  //Proccess TextFiled as a Image
    GraphicsBox *m_cont;
    QString m_cond_printing;
    QString m_cond_higlighting;
    QString m_imgFormat;
    SPtrQChart m_chart;
    void fillSeriesTbl();

private slots:
    void loadImage();
    void saveImage();
    void openProperty();
    void conditionalToggled(bool value);
    void backGroundToggled(bool value);
    void conditionChanged(const QString &text);
    void chooseColor();
    void fontSelect();
    void encodeHighLightingString();
    void decodeHighLightingString();
    void autoFillData(bool value);
    void setScaledContents(bool value);
    void update_preview();
    void textDirection();
    void chartTypeChanged(int index);
    void removeSeries();
    void addSeries();
    void seriesValue();
    void seriesDoubleClicked(QTableWidgetItem *item);
    int showSeries(QAbstractSeries *abstrSeries, int barSetNo);

};
