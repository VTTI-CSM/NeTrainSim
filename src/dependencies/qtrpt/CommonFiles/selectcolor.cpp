/*
Name: CommonFiles
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

#include "selectcolor.h"

SelectColor::SelectColor(QWidget *parent, QString rgbColor)
: QWidget(parent)
{
    auto colorBox = new QWidget(this);
    colorBox->setObjectName("colorBox");
    QString stl = "QWidget#colorBox {;"
                  "border-width:1px;"
                  "border-style:solid;"
                  "border-color:rgba(0,0,0,255);"
                  "border-top-color:rgba(0,0,0,255);"
                  "border-left-color:rgba(0,0,0,255);"
                  "border-right-color:rgba(0,0,0,255);"
                  "border-bottom-color:rgba(0,0,0,255);"
                  "color:rgba(0,0,0,255);"
                  "background-color:"+rgbColor+";"
            "}";
    colorBox->setStyleSheet(stl);

    button = new QToolButton(this);
    vLayout = new QHBoxLayout(this);
    vLayout->addWidget(colorBox);
    vLayout->addWidget(button);
    vLayout->setContentsMargins(0,0,0,0);
    this->setLayout(vLayout);
}

QString SelectColor::getBackGroundColor()
{
    auto colorBox = this->findChild<QWidget *>("colorBox");
    QString str1 = colorBox->styleSheet();
    int start; int end;
    start = str1.indexOf(";background-color:",0,Qt::CaseInsensitive);
    end = str1.indexOf(";",start+1,Qt::CaseInsensitive);

    QString tmp = str1.mid(start+1,end-start-1);
    start = tmp.indexOf("rgba",0,Qt::CaseInsensitive);
    tmp = tmp.mid(start);
    if (start == -1)
        tmp = "";
    if (tmp == "#ffffff")
        tmp = "rgba(255,255,255,0)";
    return tmp;
}

void SelectColor::setBackGroundColor(QString rgbColor)
{
    QString stl = "QWidget#colorBox {;"
                  "border-width:1px;"
                  "border-style:solid;"
                  "border-color:rgba(0,0,0,255);"
                  "border-top-color:rgba(0,0,0,255);"
                  "border-left-color:rgba(0,0,0,255);"
                  "border-right-color:rgba(0,0,0,255);"
                  "border-bottom-color:rgba(0,0,0,255);"
                  "color:rgba(0,0,0,255);"
                  "background-color:"+rgbColor+";"
            "}";
    auto colorBox = this->findChild<QWidget *>("colorBox");
    colorBox->setStyleSheet(stl);
}

void SelectColor::setMargins(QMargins margins)
{
    vLayout->setContentsMargins(margins);
}
