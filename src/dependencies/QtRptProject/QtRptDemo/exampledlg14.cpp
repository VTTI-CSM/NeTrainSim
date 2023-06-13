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

#include "exampledlg14.h"
#include "ui_exampledlg14.h"

ExampleDlg14::ExampleDlg14(QWidget *parent)
: QDialog(parent), ui(new Ui::ExampleDlg14)
{
    ui->setupUi(this);
    QObject::connect(ui->btnPrint, SIGNAL(clicked()), this, SLOT(print()));
}

void ExampleDlg14::print()
{
    auto report = QtRPT::createSPtr(this);

    //Make a page of report
    RptPageObject *page = new RptPageObject();
    page->pageNo=0;
    report->pageList.append(page); //Append page to the report

    //Make a ReportTitleBand
    RptBandObject *band1 = new RptBandObject();
    band1->name = "ReportTitle";
    band1->height = 80;
    band1->type = ReportTitle;
    page->addBand(band1); //Append band to the page

    //Make a field
    RptFieldObject *field1 = new RptFieldObject();
    field1->name = "field1";
    field1->fieldType = Text;
    field1->rect.setTop(0);
    field1->rect.setLeft(0);
    field1->rect.setHeight(60);
    field1->rect.setWidth(700);
    field1->font.setBold(true);
    field1->font.setPointSize(24);
    field1->aligment = Qt::AlignCenter;
    field1->textWrap = 1;
    field1->value = "Creation of the report from user application without XML file";
    field1->borderLeft = Qt::white;
    field1->borderRight = Qt::white;
    field1->borderBottom = Qt::white;
    field1->borderTop = Qt::white;
    band1->addField(field1);  //Append field to the ReportTitleBand

    //Make a MasterHeaderBand
    RptBandObject *band2 = new RptBandObject();
    band2->name = "MasterHeaderBand";
    band2->height = 30;
    band2->type = MasterHeader;
    page->addBand(band2); //Append band to the page

    //Make a field
    RptFieldObject *h1 = new RptFieldObject();
    h1->name = "h1";
    h1->fieldType = Text;
    h1->rect.setTop(0);
    h1->rect.setLeft(0);
    h1->rect.setHeight(30);
    h1->rect.setWidth(150);
    h1->value = "Header 1";
    h1->font.setBold(true);
    h1->setDefaultBackgroundColor(Qt::lightGray); //Set default background color
    h1->aligment = Qt::AlignCenter;
    band2->addField(h1);  //Append field to the MasterHeaderBand

    RptFieldObject *h2 = new RptFieldObject();
    h2->name = "h2";
    h2->fieldType = Text;
    h2->rect.setTop(0);
    h2->rect.setLeft(149);
    h2->rect.setHeight(30);
    h2->rect.setWidth(150);
    h2->value = "Header 2";
    h2->font.setBold(true);
    h2->setDefaultBackgroundColor(Qt::lightGray); //Set default background color
    h2->aligment = Qt::AlignCenter;
    band2->addField(h2);  //Append field to the MasterHeaderBand

    RptFieldObject *h3 = new RptFieldObject();
    h3->name = "h3";
    h3->fieldType = Text;
    h3->rect.setTop(0);
    h3->rect.setLeft(298);
    h3->rect.setHeight(30);
    h3->rect.setWidth(150);
    h3->value = "Header 3";
    h3->font.setBold(true);
    h3->setDefaultBackgroundColor(Qt::lightGray); //Set default background color
    h3->aligment = Qt::AlignCenter;
    band2->addField(h3);  //Append field to the MasterHeaderBand

    RptFieldObject *h4 = new RptFieldObject();
    h4->name = "h4";
    h4->fieldType = Text;
    h4->rect.setTop(0);
    h4->rect.setLeft(447);
    h4->rect.setHeight(30);
    h4->rect.setWidth(150);
    h4->value = "Header 4";
    h4->font.setBold(true);
    h4->setDefaultBackgroundColor(Qt::lightGray); //Set default background color
    h4->aligment = Qt::AlignCenter;
    band2->addField(h4);  //Append field to the MasterHeaderBand

    //Make a MasterDataBand
    RptBandObject *band3 = new RptBandObject();
    band3->name = "MasterData";
    band3->height = 30;
    band3->type = MasterData;
    page->addBand(band3); //Append band to the page

    //Make a field
    RptFieldObject *c1 = new RptFieldObject();
    c1->name = "c1";
    c1->fieldType = Text;
    c1->rect.setTop(0);
    c1->rect.setLeft(0);
    c1->rect.setHeight(30);
    c1->rect.setWidth(150);
    band3->addField(c1);  //Append field to the MasterDataBand

    RptFieldObject *c2 = new RptFieldObject();
    c2->name = "c2";
    c2->fieldType = Text;
    c2->rect.setTop(0);
    c2->rect.setLeft(149);
    c2->rect.setHeight(30);
    c2->rect.setWidth(150);
    band3->addField(c2);  //Append field to the MasterDataBand

    RptFieldObject *c3 = new RptFieldObject();
    c3->name = "c3";
    c3->fieldType = Text;
    c3->rect.setTop(0);
    c3->rect.setLeft(298);
    c3->rect.setHeight(30);
    c3->rect.setWidth(150);
    band3->addField(c3);  //Append field to the MasterDataBand

    RptFieldObject *c4 = new RptFieldObject();
    c4->name = "c4";
    c4->fieldType = Text;
    c4->rect.setTop(0);
    c4->rect.setLeft(447);
    c4->rect.setHeight(30);
    c4->rect.setWidth(150);
    band3->addField(c4);  //Append field to the MasterDataBand

    //Make a PageFooterBand
    RptBandObject *band4 = new RptBandObject();
    band4->name = "PageFooterBand";
    band4->height = 50;
    band4->type = PageFooter;
    page->addBand(band4); //Append band to the page

    RptFieldObject *pf = new RptFieldObject();
    pf->name = "pf";
    pf->fieldType = Text;
    pf->borderLeft = Qt::white;
    pf->borderRight = Qt::white;
    pf->borderBottom = Qt::white;
    pf->borderTop = Qt::white;
    pf->font.setBold(true);
    pf->rect.setTop(0);
    pf->rect.setLeft(330);
    pf->rect.setHeight(30);
    pf->rect.setWidth(150);
    pf->value = "<Page> of <TotalPages>";
    band4->addField(pf);  //Append field to the PageFooterBand

    QObject::connect(report.data(), SIGNAL(setField(RptFieldObject &)), this, SLOT(setField(RptFieldObject &)));
    QObject::connect(report.data(), SIGNAL(setDSInfo(DataSetInfo &)),
                     this, SLOT(setDSInfo(DataSetInfo &)));


    report->printExec();
}

void ExampleDlg14::setDSInfo(DataSetInfo &dsInfo)
{
    dsInfo.recordCount = 4;
}

void ExampleDlg14::setField(RptFieldObject &fieldObject)
{
    if (fieldObject.name == "c1") {
        fieldObject.value = "Column 1 Row "+QString::number(fieldObject.recNo()+1);
        if (fieldObject.recNo() == 0)
            fieldObject.backgroundColor = Qt::yellow;
        if (fieldObject.recNo() == 1)
            fieldObject.backgroundColor = Qt::magenta;
        if (fieldObject.recNo() == 2)
            fieldObject.backgroundColor = Qt::cyan;
        if (fieldObject.recNo() == 3)
            fieldObject.backgroundColor = Qt::green;
    }
    if (fieldObject.name == "c2") {
        fieldObject.value = "Column 2 Row "+QString::number(fieldObject.recNo()+1);
        if (fieldObject.recNo() == 1)
            fieldObject.backgroundColor = Qt::yellow;
        if (fieldObject.recNo() == 2)
            fieldObject.backgroundColor = Qt::magenta;
        if (fieldObject.recNo() == 3)
            fieldObject.backgroundColor = Qt::cyan;
        if (fieldObject.recNo() == 0)
            fieldObject.backgroundColor = Qt::green;
    }
    if (fieldObject.name == "c3") {
        fieldObject.value = "Column 3 Row "+QString::number(fieldObject.recNo()+1);
        if (fieldObject.recNo() == 2)
            fieldObject.backgroundColor = Qt::yellow;
        if (fieldObject.recNo() == 3)
            fieldObject.backgroundColor = Qt::magenta;
        if (fieldObject.recNo() == 0)
            fieldObject.backgroundColor = Qt::cyan;
        if (fieldObject.recNo() == 1)
            fieldObject.backgroundColor = Qt::green;
    }
    if (fieldObject.name == "c4") {
        fieldObject.value = "Column 4 Row "+QString::number(fieldObject.recNo()+1);
        if (fieldObject.recNo() == 3)
            fieldObject.backgroundColor = Qt::yellow;
        if (fieldObject.recNo() == 0)
            fieldObject.backgroundColor = Qt::magenta;
        if (fieldObject.recNo() == 1)
            fieldObject.backgroundColor = Qt::cyan;
        if (fieldObject.recNo() == 2)
            fieldObject.backgroundColor = Qt::green;
    }
}

ExampleDlg14::~ExampleDlg14()
{
    delete ui;
}
