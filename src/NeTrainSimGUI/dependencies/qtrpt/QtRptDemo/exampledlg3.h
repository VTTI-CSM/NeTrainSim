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

#ifndef EXAMPLEDLG3_H
#define EXAMPLEDLG3_H

#include <QDialog>
#include "qtrpt.h"

namespace Ui {
class ExampleDlg3;
}

class ExampleDlg3 : public QDialog
{
    Q_OBJECT
    
public:
    explicit ExampleDlg3(QWidget *parent = 0);
    ~ExampleDlg3();
    
private:
    Ui::ExampleDlg3 *ui;
    QPrintPreviewWidget *preview;
    QPrinter *printer;
    QtRPT *report;

private slots:
    void updatePreview();
    void setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage);
};

#endif // EXAMPLEDLG3_H
