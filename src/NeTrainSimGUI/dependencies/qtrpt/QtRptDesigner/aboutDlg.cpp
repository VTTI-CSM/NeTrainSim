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

#include "aboutDlg.h"

AboutDlg::AboutDlg(QWidget *parent)
: QDialog(parent)
{
    this->setParent(parent);
    this->setWindowTitle(tr("About QtRptDesigner"));
    QSize MaxSize(450, 550);
    QSize MinSize(450, 550);
    setMaximumSize(MaxSize);
    setMinimumSize(MinSize);

    auto btnOK = new QPushButton(this);
    btnOK->setText("OK");
    QObject::connect(btnOK, SIGNAL(clicked()), this, SLOT(close()));

    auto labImg = new QLabel(this);
    labImg->setPixmap(QPixmap(":/new/prefix1/images/Logo128.png"));
    QString lbl1 = "<h2><b><p style='color:#0033FF'>"+QApplication::applicationName()+"</p></b></h2>"+
                  tr("Version: ")+QApplication::applicationVersion()+"<br>"+
                  tr("Programmer: Aleksey Osipov")+"<br>"+
                  tr("Web Site: ") + "<a href='http://www.aliks-os.tk'>http://www.aliks-os.tk</a>"+"<br>"+
                  tr("E-mail: ") + "aliks-os@ukr.net"+"<br>"+
                  "<a href='https://qtrpt.sourceforge.io'>https://qtrpt.sourceforge.io</a>"+"<br>"+
                  tr("2012-2021 years")+"<br><br>";
    QString lbl2 = "<b>"+tr("Thanks for donation and project supporting:")+"</b>"+
                  "<ul>"+
                  "<li>"+tr("Sailendram")+"</li>"+
                  "<li>"+tr("Bill Neiderhiser")+"</li>"+
                  "</ul>"+
                  "<b>"+tr("Thanks for project developing:")+"</b>"+
                  "<ul>"+
                  "<li>"+tr("Lukas Lalinsky for DBmodel")+"</li>"+
                  "<li>"+tr("Norbert Schlia for help in developing")+"</li>"+
                  "<li>"+tr("Muhamad Bashir Al-Noimi for Arabic translation")+"</li>"+
                  "<li>"+tr("Luis Brochado for Portuguese translation")+"</li>"+
                  "<li>"+tr("Li Wei for Chinese translation")+"</li>"+
                  "<li>"+tr("Laurent Guilbert for French translation")+"</li>"+
                  "<li>"+tr("David Heremans for Dutch translation")+"</li>"+
                  "<li>"+tr("Mirko Marx for German translation")+"</li>"+
                  "<li>"+tr("Manuel Soriano for Spanish translation")+"</li>"+
                  "<li>"+tr("Bagavathikumar for Tamil translation")+"</li>"+
                  "<li>"+tr("Giulio Macchieraldo for Italian translation")+"</li>"+
                  "</ul>";
    auto lab1 = new QLabel(lbl1, this);
    auto lab2 = new QLabel(lbl2, this);
    QObject::connect(lab1, &QLabel::linkActivated, [=](const QString url) { QDesktopServices::openUrl(QUrl(url)); });

    auto hLayout2 = new QHBoxLayout;
    hLayout2->addWidget(labImg);
    hLayout2->addWidget(lab1);
    hLayout2->addStretch();

    auto hLayout1 = new QHBoxLayout;
    hLayout1->addStretch();
    hLayout1->addWidget(btnOK);
    hLayout1->addStretch();

    auto vLayout = new QVBoxLayout;
    vLayout->addLayout(hLayout2);
    vLayout->addWidget(lab2);
    vLayout->addStretch();
    vLayout->addLayout(hLayout1);
    this->setLayout(vLayout);
}
