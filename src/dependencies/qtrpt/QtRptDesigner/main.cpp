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

#include <QApplication>
#include <QDebug>
#include "mainwindow.h"


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setApplicationName(QApplication::tr("QtRptDesigner"));
    a.setOrganizationName("Aleksey Osipov");
    a.setOrganizationDomain("https://sourceforge.net/projects/qtrpt/");  //projects web page
    a.setApplicationVersion("3.0.0");

    QTranslator qTranslator;
    QString locale = QLocale::system().name(); //reading system locale
    QString loSettings;
    QSettings settings(QCoreApplication::applicationDirPath()+"/setting.ini",QSettings::IniFormat);
    #if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        settings.setIniCodec("UTF-8");
    #endif
    settings.beginGroup("language");
    loSettings = settings.value("language").toString();
    settings.endGroup();

    if (loSettings != "")//if language is set then ignore system locale
        locale = loSettings;
    //! NOTE: windows and linux system locale are different for serbian language, sr_RS is latin not cyril, this differenc is corrected in code

    if ( qTranslator.load(":/language/i18n/QtRprtDesigner_"+locale+".qm") ) { a.installTranslator(&qTranslator); }

    //NOTE: Arabic language is the same for all Arabic countries (ex. ar_SY = ar_LY) for that we've to use language name instaed of locale name

    if (locale.left(2) == "ar") {
        a.setLayoutDirection(Qt::RightToLeft);
    }

    QStringList args = a.arguments();
    MainWindow w(args);

    if (w.property("AllowStart").toBool()) {
        if (args.count() > 1) {
            w.runSilentMode();
            a.exit(0);
            return 0;
        } else
            w.show();

        return a.exec();
    } else {
        a.exit(0);
        return 0;
    }
}
