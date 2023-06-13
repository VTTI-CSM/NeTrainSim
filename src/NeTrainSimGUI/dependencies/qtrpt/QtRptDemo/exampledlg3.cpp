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

#include "exampledlg3.h"
#include "ui_exampledlg3.h"
#include <QDir>
#include <QDebug>

ExampleDlg3::ExampleDlg3(QWidget *parent)
: QDialog(parent), ui(new Ui::ExampleDlg3)
{
    ui->setupUi(this);
    ui->edtFirstName->setText("Aleksey");
    ui->edtLastName->setText("Osipov");
    ui->edtEmail->setText("aliks-os@ukr.net");
    connect(ui->btnUpdatePreview, SIGNAL(clicked()), this, SLOT(updatePreview()));

    QDir dir(qApp->applicationDirPath());
    #if defined(Q_OS_MAC)
        dir.cd(QFile::decodeName("../Resources"));
    #endif

    QString fileName = dir.absolutePath()+"/examples_report/example3.xml";
    report = new QtRPT(this);
    report->loadReport(fileName);

    QObject::connect(report, SIGNAL(setValue(const int, const QString, QVariant&, const int)),
                     this, SLOT(setValue(const int, const QString, QVariant&, const int)));

    printer = new QPrinter;
    printer->setOutputFormat(QPrinter::PdfFormat);
    printer->setPageOrientation(QPageLayout::Portrait);
    printer->setPageSize(QPageSize(QPageSize::A4));
    printer->setFullPage(true);

    preview = new QPrintPreviewWidget(printer, this);
    ui->widget->layout()->addWidget(preview);
    connect(preview, SIGNAL(paintRequested(QPrinter*)), report, SLOT(printPreview(QPrinter*)));
    preview->show();
    preview->fitInView();

    updatePreview();
}

void ExampleDlg3::setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage)
{
    Q_UNUSED(recNo);
    Q_UNUSED(reportPage);

    if (paramName == "FirstName")
        paramValue = ui->edtFirstName->text();
    if (paramName == "LastName")
        paramValue = ui->edtLastName->text();
    if (paramName == "Email")
        paramValue = ui->edtEmail->text();
}

void ExampleDlg3::updatePreview()
{
    preview->updatePreview();
}

ExampleDlg3::~ExampleDlg3()
{
    delete ui;
}
