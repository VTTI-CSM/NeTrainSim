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

#ifndef ABOUTDLG_H
#define ABOUTDLG_H

#include <QWidget>
#include <QDialog>
#if QT_VERSION >= 0x50000
    #include <QtWidgets>
#else
    #include <QtGui>
#endif

class AboutDlg : public QDialog {
    Q_OBJECT
public:
    explicit AboutDlg(QWidget *parent = 0);

signals:

};

#endif // ABOUTDLG_H
