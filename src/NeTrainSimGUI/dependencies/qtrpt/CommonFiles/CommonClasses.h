/*
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

#include <QMap>
#include <QMetaEnum>
#include <QEvent>
#if QT_VERSION >= 0x50000
    #include <QtMath>
#else
    #include <QtCore/qmath.h>
#endif
#include <QObject>
#include <QWidget>

QString double2MoneyUKR(double n, int currency = 1);
QString double2MoneyRUS(double n, int currency = 2);
QString double2MoneyGER(double number, bool bAdditional = false);
QString double2MoneyENG(double number);
QString double2MoneyESP_Group(int _siGroup, char *_tscGroup, int _siGValue);
QString double2MoneyESP(double _dbValue, int _blDecimals);
QString double2MoneyFrench(int number, int language);
QString double2MoneyFrenchBE(double number, bool bAdditional /*= false*/);
QString double2MoneyFrenchFR(double number, bool bAdditional /*= false*/);
QString double2MoneyFrenchCH(double number, bool bAdditional /*= false*/);
QString double2MoneyITA(double n);
QString double2MoneyAR(double n);
QString double2MoneyTR(double gelenRakam);
QString yaziyaCevir(int gelenInt);

QString double2Money(double n, QString lang);
QString colorToString(QColor color);
QColor colorFromString(QString value);
QString eventType(QEvent *ev);

void drawText(QPainter *painter, QString text, QFont font, QColor fontColor, QRectF rcT, int flags, int renderingMode, int textRotate);
QString plainText2html(QString text, QRectF rcT, int flags, QFont font, QColor fontColor, int textRotate);

/*pointer to QVariant and back
http://blog.bigpixel.ro/2010/04/storing-pointer-in-qvariant/
usage

MyClass *p;
QVariant v = VPtr<MyClass>::asQVariant(p);

MyClass *p1 = VPtr<MyClass>::asPtr(v);
*/
template <class T> class VPtr
{
public:
    static T* asPtr(QVariant v)
    {
    return  (T *) v.value<void *>();
    }

    static QVariant asQVariant(T* ptr)
    {
    return QVariant::fromValue((void *) ptr);
    }
};

