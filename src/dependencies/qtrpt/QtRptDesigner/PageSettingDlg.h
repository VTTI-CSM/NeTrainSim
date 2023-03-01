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

#ifndef PAGESETTINGDLG_H
#define PAGESETTINGDLG_H

#include <QDialog>

namespace Ui {
    class PageSettingDlg;
}

struct PageSetting {
    QString pageFormat;
    float marginsLeft;
    float marginsRight;
    float marginsTop;
    float marginsBottom;
    float pageWidth;
    float pageHeight;
    int pageOrientation;
    bool border;
    bool watermark;
    double watermarkOpacity;
    QPixmap watermarkPixmap;
    int borderWidth;
    QString borderColor;
    QString borderStyle;
};

class PageSettingDlg : public QDialog
{
    Q_OBJECT

public:
    explicit PageSettingDlg(QWidget *parent = nullptr);
    ~PageSettingDlg();
    void showThis(PageSetting pageSetting);
    PageSetting pageSetting;

private:
    static const int w_A3 = 1188;
    static const int h_A3 = 16800;
    static const int w_A4 = 840;
    static const int h_A4 = 1188;
    static const int w_A5 = 592;
    static const int h_A5 = 840;
    static const int w_Lt = 863;
    static const int h_Lt = 1118;
    double koef;
    Ui::PageSettingDlg *ui;
    QString strColor;
    QPixmap m_watermarkPixmap;
    void saveSettings();
    void scaleImage();

private slots:
    void changeOrientation();
    void pageSizeChanged(int index);
    void selectColor();
    void changeWatermark();

};

#endif // PAGESETTINGDLG_H
