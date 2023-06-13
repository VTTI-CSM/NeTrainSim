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

#include "CommonClasses.h"
#include "numbertoarabicword.h"
#include <QTextDocument>
#include <QPainter>
#include <QDebug>

QString double2MoneyUKR(double n, int currency)
{
    static QString cap[4][10] =	{
        {
            "",
            "один ",
            "два ",
            "три ",
            "чотири ",
            "п'ять ",
            "шість ",
            "сім ",
            "вісім ",
            "дев'ять "
        },
        {
            "",
            "одна ",
            "дві ",
            "три ",
            "чотири ",
            "п'ять ",
            "шість ",
            "сім ",
            "вісім ",
            "дев'ять "
        },
        {
            "",
            "",
            "двадцять ",
            "тридцять ",
            "сорок ",
            "п'ятдесят ",
            "шістдесят ",
            "сімдесят ",
            "вісімдесят ",
            "дев'яносто "
        },
        {
            "",
            "сто ",
            "двісти ",
            "триста ",
            "чотириста ",
            "п'ятсот ",
            "шістсот ",
            "сімсот ",
            "вісімсот ",
            "дев'ятсот "
        }
    };

    static QString cap2[10] = {
        "десять ",
        "одинадцять ",
        "дванадцять ",
        "тринадцять ",
        "чотирнадцять ",
        "п'ятнадцять ",
        "шістнадцять ",
        "сімнадцять ",
        "вісімнадцять ",
        "дев'ятнадцять "
    };

    static QString cap3[5][3] = {
        {
            "копійка",
            "копійки",
            "копійок"
        },
        {
            "гривня ",
            "гривні ",
            "гривень "
        },
        {
            "тисяча ",
            "тисячі ",
            "тисяч "
        },
        {
            "мілльон ",
            "мильони ",
            "мільонів "
        },
        {
            "мілліард ",
            "мільярди ",
            "мільярдів "
        }
    };

    if (currency == 0) {
        cap3[0][0] = "";
        cap3[0][1] = "";
        cap3[0][2] = "";

        cap3[1][0] = ", ";
        cap3[1][1] = ", ";
        cap3[1][2] = ", ";
    }
    QString s;

    long nn = ((int)(n)*10000 + (int)(n*1000) % 1000 + 5 ) / 10;

    long lo = 0;
    long hi = nn;
    int r[3];
    int rod;

    for ( int nPor = 0; true; ++nPor ) {
        QString olds = s;
        s = "";

        lo = hi%1000;
        hi = hi/1000;

        if ((nPor == 1) && (lo == 0) && (hi == 0)) {
            s = "ноль " + cap3[1][2] + olds;
            break;
        }

        for ( int nRaz = 0; nRaz < 3; ++nRaz ) {
            r[nRaz] = lo%10;
            lo      = lo/10;
        }

        switch ( nPor ) {
            case 0:
            case 1:
            case 2:
                rod = 1;
                break;
            default:
                rod = 0;
        }

        QString capt;
        if (r[1] != 1) {
            if (r[0] == 1)
                capt = cap3[nPor][0];
            else if ((r[0] > 1 ) && (r[0] < 5))
                capt = cap3[nPor][1];
        }
        if (capt.isEmpty())
            capt = cap3[nPor][2];

        if (nPor == 0) {
            s = QString::number(r[1]) + QString::number(r[0]) + " " + capt;
            continue;
        }

        s += cap[2+1][r[2]];
        s += ((r[1] == 1) ? cap2[r[0]] : cap[1+1][r[1]]+cap[0+rod][r[0]])+capt;
        if (hi == 0)  //Если не нужна часть после запятой
            s += olds;

        if (hi == 0 && nPor >= 1)
            break;
    }
    QString up(s[0]);
    s.replace(0,1,up);
    //return s.replace(" ,",",");
    return s.replace(",","");
}

QString double2MoneyRUS(double n, int currency) {
    QString v1,v2,v3,s1,s2,s3;
    switch(currency) {
        case 0: {  //БЕЗ ВАЛЮТЫ
            v1 = QString::fromUtf8(", ");
            v2 = QString::fromUtf8(", ");
            v3 = QString::fromUtf8(", ");
            s1 = QString::fromUtf8("один ");
            s2 = QString::fromUtf8("два ");
            break;
        }
        case 1: {  //ГРИВНА
            v1 = QString::fromUtf8("гривна, ");
            v2 = QString::fromUtf8("гривны, ");
            v3 = QString::fromUtf8("гривен, ");
            s1 = QString::fromUtf8("одна ");
            s2 = QString::fromUtf8("две ");
            break;
        }
        case 2: {  //РУБЛЬ
            v1 = QString::fromUtf8("рубль, ");
            v2 = QString::fromUtf8("рубля, ");
            v3 = QString::fromUtf8("рублей, ");
            s1 = QString::fromUtf8("один ");
            s2 = QString::fromUtf8("два ");
            break;
        }
    }

    if ( n > 999999999999.99 )
        return QString::fromUtf8("Очень много денег ...");

    static QString cap[4][10] = {
        {
            "",
            s1,
            s2,
            QString::fromUtf8("три "),
            QString::fromUtf8("четыре "),
            QString::fromUtf8("пять "),
            QString::fromUtf8("шесть "),
            QString::fromUtf8("семь "),
            QString::fromUtf8("восемь "),
            QString::fromUtf8("девять ")
        },
        {
            "",
            QString::fromUtf8("одна "),
            QString::fromUtf8("две "),
            QString::fromUtf8("три "),
            QString::fromUtf8("четыре "),
            QString::fromUtf8("пять "),
            QString::fromUtf8("шесть "),
            QString::fromUtf8("семь "),
            QString::fromUtf8("восемь "),
            QString::fromUtf8("девять ")
        },
        {
            "",
            "",
            QString::fromUtf8("двадцать "),
            QString::fromUtf8("тридцать "),
            QString::fromUtf8("сорок "),
            QString::fromUtf8("пятьдесят "),
            QString::fromUtf8("шестьдесят "),
            QString::fromUtf8("семьдесят "),
            QString::fromUtf8("восемьдесят "),
            QString::fromUtf8("девяносто ")
        },
        {
            "",
            QString::fromUtf8("сто "),
            QString::fromUtf8("двести "),
            QString::fromUtf8("триста "),
            QString::fromUtf8("четыреста "),
            QString::fromUtf8("пятьсот "),
            QString::fromUtf8("шестьсот "),
            QString::fromUtf8("семьсот "),
            QString::fromUtf8("восемьсот "),
            QString::fromUtf8("девятьсот ")
        }
    };

    static QString cap2[10] = {
        QString::fromUtf8("десять "),
        QString::fromUtf8("одиннадцать "),
        QString::fromUtf8("двенадцать "),
        QString::fromUtf8("тринадцать "),
        QString::fromUtf8("четырнадцать "),
        QString::fromUtf8("пятнадцать "),
        QString::fromUtf8("шестнадцать "),
        QString::fromUtf8("семнадцать "),
        QString::fromUtf8("восемнадцать "),
        QString::fromUtf8("девятнадцать ")
    };

    static QString cap3[5][3] = {
        {
            QString::fromUtf8("копейка "),
            QString::fromUtf8("копейки "),
            QString::fromUtf8("копеек ")
        },
        {
            v1,
            v2,
            v3
        },
        {
            QString::fromUtf8("тысяча "),
            QString::fromUtf8("тысячи "),
            QString::fromUtf8("тысяч ")
        },
        {
            QString::fromUtf8("миллион "),
            QString::fromUtf8("милиона "),
            QString::fromUtf8("миллионов ")
        },
        {
            QString::fromUtf8("миллиард "),
            QString::fromUtf8("миллиарда "),
            QString::fromUtf8("миллиардов ")
        }
    };

    if (currency == 0) {
        cap3[0][0] = "";
        cap3[0][1] = "";
        cap3[0][2] = "";
    }
    QString s;

    qlonglong nn = ((qlonglong)(n)*10000 + (qlonglong)(n*1000) % 1000 + 5 ) / 10;

    qlonglong lo = 0;
    qlonglong hi = nn;
    int r[3];
    int rod;

    for ( int nPor = 0; true; ++nPor ) {
        QString olds = s;
        s = "";

        lo = hi%1000;
        hi = hi/1000;

        if ((nPor == 1) && (lo == 0) && (hi == 0)) {
            s = QString::fromUtf8("ноль ") + cap3[1][2] + olds;
            break;
        }

        for ( int nRaz = 0; nRaz < 3; ++nRaz ) {
            r[nRaz] = lo%10;
            lo      = lo/10;
        }

        switch ( nPor ) {
            case 0:
            case 1:
            case 2:
                rod = 1;
                break;
            default:
                rod = 0;
        }

        QString capt;
        if (r[1] != 1) {
            if (r[0] == 1)
                capt = cap3[nPor][0];
            else if ((r[0] > 1 ) && (r[0] < 5))
                capt = cap3[nPor][1];
        }
        if (capt.isEmpty())
            capt = cap3[nPor][2];

        if (nPor == 0) {
            s = QString::number(r[1]) + QString::number(r[0]) + " " + capt;
            continue;
        }
        if (nPor == 1)
            rod = 0;

        s += cap[2+1][r[2]];
        s += ((r[1] == 1) ? cap2[r[0]] : cap[1+1][r[1]]+cap[0+rod][r[0]])+capt;
        s += olds;

        if ((hi == 0) && (nPor >= 1))
            break;
    }
    QString up(s[0]);
    s.replace(0,1,up);
    if ( n < 0 )
        return QString::fromUtf8("Минус ")+s;
    return s.replace(" ,",",");
}


//Based on https://github.com/icelander/number_to_text
//No license encumbrance.
QString double2MoneyENG(double number) {
    static QMap<double, QString> numbers;

    //Only initialize once
    if (numbers.isEmpty()) {
        numbers[0] = "zero";
        numbers[1] = "one";
        numbers[2] = "two";
        numbers[3] = "three";
        numbers[4] = "four";
        numbers[5] = "five";
        numbers[6] = "six";
        numbers[7] = "seven";
        numbers[8] = "eight";
        numbers[9] = "nine";
        numbers[10] = "ten";
        numbers[11] = "eleven";
        numbers[12] = "twelve";
        numbers[13] = "thirteen";
        numbers[14] = "fourteen";
        numbers[15] = "fifteen";
        numbers[16] = "sixteen";
        numbers[17] = "seventeen";
        numbers[18] = "eighteen";
        numbers[19] = "nineteen";
        numbers[20] = "twenty";
        numbers[30] = "thirty";
        numbers[40] = "forty";
        numbers[50] = "fifty";
        numbers[60] = "sixty";
        numbers[70] = "seventy";
        numbers[80] = "eighty";
        numbers[90] = "ninety";
    }

    static QMap<uint, QString> powers;

    //Only initialize once
    if (powers.isEmpty()) {
        powers[2] = "hundred";
        powers[3] = "thousand";
        powers[6] = "million";
        powers[9] = "billion";
    }

    QString output;

    // Thanks to Numael Garay for bug fixing
    if (number < 21) {
        double remainder = number - qFloor(number);

        if (remainder > 0) {
            output = numbers[qFloor(number)];
        } else {
            output = numbers[number];
        }
    } else if (number < 100) {
        output = numbers[10 * qFloor(number / 10)];
        int remainder = (int)number % 10;

        if (remainder > 0)
            output += "-" + double2MoneyENG(remainder);
    } else {
        uint power = 2;
        uint place = 0;
        QString powerString;

        //QMap::keys is ordered
        foreach (uint pow, powers.keys()) {
            uint place_value = qPow(10, pow);
            uint tmp_place = qFloor(number / place_value);
            if (tmp_place < 1)
                break;

            place = tmp_place;
            power = pow;
            powerString = powers[pow];
        }

        if (power > 0) {
            output = double2MoneyENG(place) + " " + powerString;
            double remainder = (int)number % (int)double(qPow(10, power));

            if (remainder > 0)
            output += " " + double2MoneyENG(remainder);
        }
    }

    return output;
}

//Thanks to Norbert Schlia
QString double2MoneyGER(double number, bool bAdditional /*= false*/)
{
    Q_UNUSED(bAdditional)

    static QMap<double, QString> numbers;

    //Only initialize once
    if (numbers.isEmpty()) {
        numbers[0] = "null";
        numbers[1] = "ein";
        numbers[2] = "zwei";
        numbers[3] = "drei";
        numbers[4] = "vier";
        numbers[5] = "fünf";
        numbers[6] = "sechs";
        numbers[7] = "sieben";
        numbers[8] = "acht";
        numbers[9] = "neun";
        numbers[10] = "zehn";
        numbers[11] = "elf";
        numbers[12] = "zwölf";
        numbers[13] = "dreizehn";
        numbers[14] = "vierzehn";
        numbers[15] = "fünfzehn";
        numbers[16] = "sechzehn";
        numbers[17] = "siebzehn";
        numbers[18] = "achtzehn";
        numbers[19] = "neunzehn";
        numbers[20] = "zwanzig";
        numbers[30] = "dreissig";
        numbers[40] = "vierzig";
        numbers[50] = "fünfzig";
        numbers[60] = "sechszig";
        numbers[70] = "siebzig";
        numbers[80] = "achtzig";
        numbers[90] = "neunzig";
    }

    static QMap<uint, QString> powers;

    //Only initialize once
    if (powers.isEmpty()) {
        powers[2] = "hundert";
        powers[3] = "tausend";
        powers[6] = "millionen";
        powers[9] = "milliarden";
    }

    QString output;

    if ((long long)number == 1 && !bAdditional) {
        output = "eins";
    }
    else if (number < 21) {
        output = numbers[(long long)number];
    } else if (number < 100) {
        output = numbers[10 * qFloor(number / 10)];
        int remainder = (long long)number % 10;

        if (remainder > 0)
            output = double2MoneyGER(remainder, true) + "und" + output;
    } else {
        uint power = 2;
        uint place = 0;
        QString powerString;

        //QMap::keys is ordered
        foreach (uint pow, powers.keys()) {
            uint place_value = qPow(10, pow);
            uint tmp_place = qFloor(number / place_value);
            if (tmp_place < 1)
                break;

            place = tmp_place;
            power = pow;

            if (pow == 6 && number < 2E6) {
                powerString = "emillion";
            }
            else if (pow == 9 && number < 2E9) {
                powerString = "emilliarde";
            }
            else {
                powerString = powers[pow];
            }
        }

        if (power > 0) {
            output = double2MoneyGER(place, true) + powerString;
            double remainder = (long long)number % (long long)double(qPow(10, power));

            if (remainder > 0)
                output += double2MoneyGER(remainder, true);
        }
    }

    return output;
}

//Thanks to Manuel Soriano
#define	VALEN 16
QString double2MoneyESP_Group(int _siGroup, char *_tscGroup, int _siGValue)
{
    int			siInd1, siWk1, siValue;
    QString		stReturn, stWk1;

    static QMap<int, QString> stUnits;
    static QMap<int, QString> stDeci;
    if (stUnits.isEmpty()) {
        stUnits[0] = "cero ";
        stUnits[1] = "uno ";
        stUnits[2] = "dos ";
        stUnits[3] = "tres ";
        stUnits[4] = "cuatro ";
        stUnits[5] = "cinco ";
        stUnits[6] = "seis ";
        stUnits[7] = "siete ";
        stUnits[8] = "ocho ";
        stUnits[9] = "nueve ";
        stUnits[10] = "diez ";
        stUnits[11] = "once ";
        stUnits[12] = "doce ";
        stUnits[13] = "trece ";
        stUnits[14] = "catorce ";
        stUnits[15] = "quince ";
        stUnits[16] = "diez y seis ";
        stUnits[17] = "diez y siete ";
        stUnits[18] = "diez y ocho ";
        stUnits[19] = "diez y nueve ";

        stDeci[2] = "veinte ";
        stDeci[3] = "treinta ";
        stDeci[4] = "cuarenta ";
        stDeci[5] = "cincuenta ";
        stDeci[6] = "sesenta ";
        stDeci[7] = "setenta ";
        stDeci[8] = "ochenta ";
        stDeci[9] = "noventa ";
    }

    for (siInd1=0; siInd1 < 3; siInd1++) {
        siValue = *(_tscGroup+siInd1) - 0x30;
        stWk1 = stUnits[siValue];

        // "cero" away
        if (_siGValue == 0)	{
            stReturn.clear();
            siInd1 = 3;
            continue;
        }

        // We do not want the "cero" text in our sentence
        if (siValue == 0) {
            continue;
        }

        if (_siGValue == 1) {
            if (_siGroup == 3) {
                stReturn.clear();	// mil
                siInd1 = 3;
                continue;
            }
        }

        if (siInd1 == 0) {
            if (siValue == 1) {
                if (atoi(_tscGroup) == 100) {
                    stReturn = ::QString("cien ");
                    siInd1 = 3;
                    continue;
                } else {
                    stWk1 = ::QString("ciento ");
                }
            } else {
                stWk1 += ::QString("cientos ");
            }
        }

        if (siInd1 == 1) {
            if (siValue == 1) {
                siWk1 = (siValue * 10) + *(_tscGroup+siInd1+1) - 0x30;
                stReturn += stUnits[siWk1];
                siInd1 = 3;
                continue;
            }

            if (siValue > 1) {
                stWk1 = stDeci[siValue];
                stWk1 += ::QString("y ");
            }
        }

        if (siInd1 == 2) {
            if ((siValue == 1) && (_siGroup < 4))
                stWk1 = ::QString("un ");
        }

        stReturn += stWk1;
    }

    return stReturn;
}

/*
Thanks to Manuel Soriano
0 : Does not print the decimals
1 : Print the decimals
*/
QString double2MoneyESP(double _dbValue, int _blDecimals)
{
    QString		stValue, stReturn;

    long		slValue, slDecimals;
    int			siLen, siWk1, siValue;
    char		tscGroup[4], tscValue[VALEN];

    static QMap<int, QString> stMillos;
    if (stMillos.isEmpty()) {
        stMillos[0] = "billónes ";
        stMillos[1] = "millardos ";
        stMillos[2] = "millones ";
        stMillos[3] = "mil ";
        stMillos[4] = "";
        stMillos[5] = "";
    }

    static QMap<int, QString> stMillo;
    if (stMillo.isEmpty()) {
        stMillo[0] = "billón ";
        stMillo[1] = "millardo ";
        stMillo[2] = "millón ";
        stMillo[3] = "mil ";
        stMillo[4] = "";
        stMillo[5] = "";
    }


    stReturn.clear();
    memset(tscValue, 0x00, sizeof(tscValue));
    memset(tscValue, 0x30, sizeof(tscValue)-1);

    slValue = _dbValue * 1;
    slDecimals = (_dbValue - slValue) * 100.;

    stValue.setNum(slValue);
    siLen = stValue.length();

    memcpy(tscValue+((VALEN-siLen)-1), stValue.toLatin1().data(), siLen);

    for (siWk1=0; siWk1 < (VALEN/3); siWk1++) {
        memset(tscGroup, 0x00, sizeof(tscGroup));
        memcpy(tscGroup, tscValue+(3*siWk1), 3);
        if (strcmp(tscGroup, "000") == 0)
            continue;

        siValue = atoi(tscGroup);
        stReturn += double2MoneyESP_Group(siWk1, tscGroup, siValue);

        if (siValue == 1)
            stReturn += stMillo[siWk1];
        else
            stReturn += stMillos[siWk1];
    }

    if ((slDecimals > 0) && _blDecimals) {
        stReturn += ::QString("con ");
        stValue.setNum(slDecimals);
        siLen = stValue.length();

        memset(tscValue, 0x00, sizeof(tscValue));
        memset(tscValue, 0x30, sizeof(tscValue)-1);
        memcpy(tscValue+((VALEN-siLen)-1), stValue.toLatin1().data(), siLen);

        for (siWk1=0; siWk1 < (VALEN/3); siWk1++) {
            memset(tscGroup, 0x00, sizeof(tscGroup));
            memcpy(tscGroup, tscValue+(3*siWk1), 3);
            if (strcmp(tscGroup, "000") == 0)
                continue;

            siValue = atoi(tscGroup);
            stReturn += double2MoneyESP_Group(siWk1, tscGroup, siValue);

            if (siValue == 1)
                stReturn += stMillo[siWk1];
            else
                stReturn += stMillos[siWk1];
        }
    }

    return stReturn;
}

//Thanks to Laurent Guilbert
QString double2MoneyFrenchBE(double number, bool bAdditional /*= false*/)
{
    Q_UNUSED(bAdditional)

    int whole = (int)number;
    int precision = ((number-whole)*100)+0.5;

    if (precision > 0) {
        return double2MoneyFrench(whole,1) + " Euros " + double2MoneyFrench(precision,1) + " Centime(s)";
    } else {
        return double2MoneyFrench(whole,1) + " Euros ";
    }
}

QString double2MoneyFrenchFR(double number, bool bAdditional /*= false*/)
{
    Q_UNUSED(bAdditional)

    int whole = (int)number;
    int precision = (number - whole) * 100;

    if (precision > 0) {
        return double2MoneyFrench(whole,0) + " Euros " + double2MoneyFrench(precision,1) + " Centime(s)";
    } else {
        return double2MoneyFrench(whole,0) + " Euros ";
    }
}

QString double2MoneyFrenchCH(double number, bool bAdditional /*= false*/)
{
    Q_UNUSED(bAdditional)

    int whole = (int)number;
    int precision = (number - whole) * 100;

    if (precision > 0) {
        return double2MoneyFrench(whole,2) + " Francs " + double2MoneyFrench(precision,1) + " Centime(s)";
    } else {
        return double2MoneyFrench(whole,2) + " Francs ";
    }
}

QString double2MoneyFrench(int number, int language)
{
    QMap<double, QString> numbers;

    //Only initialize once
    if (numbers.isEmpty()) {
        numbers[0] = "zéro";
        numbers[1] = "un";
        numbers[2] = "deux";
        numbers[3] = "trois";
        numbers[4] = "quatre";
        numbers[5] = "cinq";
        numbers[6] = "six";
        numbers[7] = "sept";
        numbers[8] = "huit";
        numbers[9] = "neuf";
        numbers[10] = "dix";
        numbers[11] = "onze";
        numbers[12] = "douze";
        numbers[13] = "treize";
        numbers[14] = "quatorze";
        numbers[15] = "quinze";
        numbers[16] = "seize";
        numbers[17] = "dix-sept";
        numbers[18] = "dix-huit";
        numbers[19] = "dix-neuf";
        numbers[20] = "vingt";
        numbers[30] = "trente";
        numbers[40] = "quarante";
        numbers[50] = "cinquante";
        numbers[60] = "soixante";

        switch(language) {
            case 0: // France
                numbers[70] = "soixante-dix";
                numbers[80] = "quatre-vingt";
                numbers[90] = "quatre-vingt-dix";
                break;
            case 1: // Belgium
                numbers[70] = "septante";
                numbers[80] = "quatre-vingt";
                numbers[90] = "nonante";
                break;
            case 2: // Switzerland
                numbers[70] = "septante";
                numbers[80] = "huitante";
                numbers[90] = "nonante";
                break;
        }
    }

    QMap<uint, QString> powers;

    //Only initialize once
    if (powers.isEmpty()) {
        powers[3] = "mille";
        powers[6] = "million";
        powers[9] = "milliard";
    }

    QString output = "";

    int remainder = 0;

    if (number == 1) {
        output = "et-" + numbers[number];
    }
    else if (number < 20) {
        output = numbers[number];
    } else if (number < 100) {
        remainder = number % 10;
        // FRANCE
        if (language == 0) {
            if ((number >= 70) && (number <= 79)) {
                output = numbers[10 * (qFloor(number / 10) - 1)];
                remainder += 10;
            } else if ((number == 80) && (remainder == 0)) {
                output = numbers[10 * qFloor(number / 10)] + "s";
            } else if ((number >= 90) && (number <= 99)) {
                output = numbers[10 * (qFloor(number / 10)-1)];
                remainder += 10;
            } else {
                output = numbers[10 * qFloor(number / 10)];
            }
        }
        // BELGIUM
        if (language == 1) {
            if ((number == 80) && (remainder == 0)) {
                output = numbers[10 * qFloor(number / 10)] + "s";
            } else {
                output = numbers[10 * qFloor(number / 10)];
            }
        }
        // FRENCH SWITZERLAND
        if (language == 2) {
            output = numbers[10 * qFloor(number / 10)];
        }

        if (remainder > 0) {
            output =  output + "-" + double2MoneyFrench(remainder, language);
        }
    } else if (number < 999) {
        remainder = number % 100;
        if (floor(number / 100) == 1) {
            output = "cent-" + double2MoneyFrench(remainder, language);
        } else {
            if (remainder > 0 ) {
                output = numbers[qFloor(number / 100)] + "-cent-" + double2MoneyFrench(remainder, language);
            } else {
                output = numbers[qFloor(number / 100)] + "-cents";
            }
        }
    } else {
        uint power = 2;
        uint place = 0;
        QString powerString;

        //QMap::keys is ordered
        foreach (uint pow, powers.keys()) {
            uint place_value = qPow(10, pow);
            uint tmp_place = qFloor(number / place_value);
            if (tmp_place < 1)
                break;

            place = tmp_place;
            power = pow;

            if (pow == 6 && number < 2E6) {
                powerString = powers[pow];
            }
            else if (pow == 9 && number < 2E9) {
                powerString = powers[pow];
            }
            else {
                powerString = powers[pow];
            }
        }

        if (power > 0) {
            if (power == 3) {
                if (floor(number / 1000) == 1) {
                    output = powerString + "-";
                } else {
                    output = double2MoneyFrench(place, language) + "-" + powerString + "-";
                }
            }
            if (power == 6) {
                if (floor(number / 1000000) == 1) {
                    output = numbers[1] + "-" + powerString + "-";
                } else {
                    output = double2MoneyFrench(place, language) + "-" + powerString + "s-";
                }
            }
            if (power == 9) {
                if (floor(number / 1000000000) == 1) {
                    output = numbers[1] + "-" + powerString + "-";
                } else {
                    output = double2MoneyFrench(place, language) + "-" + powerString + "s-";
                }
            }
            remainder = (long long)number % (long long)double(qPow(10, power));

            if (remainder > 0)
                output = output + double2MoneyFrench(remainder, language);
        }
    }

    return output;
}

QString double2MoneyITA(double n)
{
    static QMap<double, QString> numbers;

    //Only initialize once
    if (numbers.isEmpty()) {
        numbers[0] = "zero";
        numbers[1] = "uno";
        numbers[2] = "due";
        numbers[3] = "tre";
        numbers[4] = "quattro";
        numbers[5] = "cinque";
        numbers[6] = "sei";
        numbers[7] = "sette";
        numbers[8] = "otto";
        numbers[9] = "nove";
        numbers[10] = "dieci";
        numbers[11] = "undici";
        numbers[12] = "dodici";
        numbers[13] = "tredici";
        numbers[14] = "quattordici";
        numbers[15] = "quindici";
        numbers[16] = "sedici";
        numbers[17] = "diciasette";
        numbers[18] = "diciotto";
        numbers[19] = "diciannove";
        numbers[20] = "venti";
        numbers[30] = "trenta";
        numbers[40] = "quaranta";
        numbers[50] = "cinquanta";
        numbers[60] = "sessanta";
        numbers[70] = "settanta";
        numbers[80] = "ottanta";
        numbers[90] = "novanta";
        numbers[1000]= "mille";
    }

    static QMap<uint, QString> powers;

    //Only initialize once
    if (powers.isEmpty()) {
        powers[2] = "cento";
        powers[3] = "mila ";
        powers[6] = " milioni ";
        powers[9] = " miliardi ";
    }

    QString output="";

    if (n < 21) {//Number from the QMap
        output = numbers[n];
    } else if (n < 100) {
        output = numbers[10 * qFloor(n / 10)];
        int remainder = (long long)n % 10;

        if (remainder > 0){
            if(remainder==1 || remainder==8)
                output.chop(1);
            output = output + double2MoneyITA(remainder);
        }
    } else {
        uint power = 2;
        uint place = 0;
        QString powerString;

        //QMap::keys is ordered
        foreach (uint pow, powers.keys()) {
            uint place_value = qPow(10, pow);
            uint tmp_place = qFloor(n / place_value);
            if (tmp_place < 1)
                break;

            place = tmp_place;
            power = pow;

            if (pow ==3 && n < 2E3) {//1000 is "mille"
                powerString = "mille";
            }
            else if (pow == 6 && n < 2E6) {// 1 000 000 in "un milione"
                powerString = "un milione ";
            }
            else if (pow == 9 && n < 2E9) {//1 000 000 000 is "un miliardo"
                powerString = "un miliardo ";
            }
            else {
                powerString = powers[pow];
            }
        }

        if (power > 0) {
            QString notOne="";
            if (place!=1) notOne= double2MoneyITA(place);//If place is 1 then powerString contains the right value
            output = notOne + powerString;
            double remainder = (long long)n % (long long)double(qPow(10, power));

            if (remainder > 0)
                output += double2MoneyITA(remainder);
        }
    }

    return output;
}

//Thanks to Dr. Mucibirahman İLBUĞA
QString double2MoneyTR(double gelenRakam)
{
    int tam;
    double kusur;
    QString takas;

    tam = gelenRakam;
    kusur = gelenRakam - tam;

    takas = QString::number(kusur, 'f', 2);

    //QString takas = QString::number(gelenRakam,'f', 2).remove(QRegExp("\\.?0+$"));

    QStringList parca = takas.split(".");


    /*
    qDebug()<< gelenRakam;
    qDebug()<< tam;
    qDebug()<< parca.last();
    */

    if(parca.last().toInt()>0)

    return yaziyaCevir(tam) + "#TL#"+yaziyaCevir(parca.last().toInt())+"#KRŞ";
    else return yaziyaCevir(tam) + "#TL#";

}

QString yaziyaCevir(int gelenInt)
{
    QString birlik[10] = {"", "BİR", "İKİ", "ÜÇ", "DÖRT", "BEŞ", "ALTI", "YEDİ", "SEKİZ", "DOKUZ"};
    QString onluk[10] = {"", "ON", "YİRMİ", "OTUZ", "KIRK", "ELLİ", "ALTMIŞ", "YETMİŞ", "SEKSEN", "DOKSAN"};
    QString yuzluk[10] = {"", "YÜZ", "İKİYÜZ", "ÜÇYÜZ", "DÖRTYÜZ", "BEŞYÜZ", "ALTIYÜZ", "YEDİYÜZ", "SEKİZYÜZ", "DOKUZYÜZ"};
    QString bolum[11] = {"BİN", "MİLYON", "MİLYAR", "TRİLYON", "KATRİLYON", "KENTİLYON", "SEKSTİLYON", "SEPTİLYON", "OKTİLYON", "NONİLYON", "DESİLYON"};



    QString gelenMetin = QString::number(gelenInt);

    QString sonuc="";


    int basamak = 1;
    int bol = 0;


    for (int i = gelenMetin.length();i>0;i--) {

        //qDebug()<<basamak;
        //qDebug()<<i;
        //qDebug()<<gelenSayi;


        switch (basamak) {
        case 1:
            //if(gelenMetin.mid(i-1,1).toInt()!=1 && gelenMetin.length()!=4)
            sonuc = birlik[gelenMetin.mid(i-1,1).toInt()] + sonuc;
            //qDebug()<<gelenMetin.mid(i-1,1).toInt();
            //qDebug()<<birlik[gelenMetin.mid(i-1,1).toInt()];
            break;
        case 2:
            sonuc = onluk[gelenMetin.mid(i-1,1).toInt()] + sonuc;
            //qDebug()<<gelenMetin.mid(i-1,1).toInt();
            //qDebug()<<onluk[gelenMetin.mid(i-1,1).toInt()];
            break;
        case 3:
            sonuc = yuzluk[gelenMetin.mid(i-1,1).toInt()] + sonuc;
            //qDebug()<<gelenMetin.mid(i-1,1).toInt();
            //qDebug()<<yuzluk[gelenMetin.mid(i-1,1).toInt()];
            break;
        }

        basamak = basamak +1;

        if(basamak > 3)
        {
            if(i!=1) sonuc = bolum[bol]+sonuc;
            basamak=1;
            bol=bol+1;
        }

    }

    if (sonuc.mid(0,6)!="BİRBİN") return sonuc; else return sonuc.mid(3);
}

//Thanks to Mohamed Glaiow <mh_glaiow@yahoo.com>
QString double2MoneyAR(double n)
{
    NumberToArabicWord number(n);
    return number.ConvertToArabic();
}

QString double2Money(double n, QString lang)
{
    if (lang == "UKR")
        return double2MoneyUKR(n,0);
    //else if (lang == "RUS")
    //    return double2MoneyRUS(n);
    else if (lang == "GER")
        return double2MoneyGER(n);
    else if (lang == "ENG")
        return double2MoneyENG(n);
    else if (lang == "ESP")
        return double2MoneyESP(n,0);
    else if (lang == "FR_FR")
        return double2MoneyFrench(n,0);
    else if (lang == "FR_BE")
        return double2MoneyFrench(n,1);
    else if (lang == "FR_CH")
        return double2MoneyFrench(n,2);
    else if (lang == "ITA")
        return double2MoneyITA(n);
    else if (lang == "AR")
        return double2MoneyAR(n);
    else if (lang == "TR")
        return double2MoneyTR(n);
    else
        return double2MoneyENG(n);
}

QString colorToString(QColor color)
{
    QString str("rgba("+
                QString::number(color.red())+","+
                QString::number(color.green())+","+
                QString::number(color.blue())+","+
                QString::number(color.alpha())+")");
    return str;
}

QColor colorFromString(QString value)
{
    //if (value == "rgba(255,255,255,0)")
    //    value = "rgba(255,255,255,255)";
    QColor color;
    if (value.isEmpty())
        return color;
    int start; int end;
    start = value.indexOf("(",0,Qt::CaseInsensitive);
    end =   value.indexOf(")",start+1,Qt::CaseInsensitive);
    QString tmp = value.mid(start+1,end-start-1);
    int v = tmp.section(",",0,0).toInt();
    color.setRed(v);
    v = tmp.section(",",1,1).toInt();
    color.setGreen(v);
    v = tmp.section(",",2,2).toInt();
    color.setBlue(v);
    v = tmp.section(",",3,3).toInt();
    color.setAlpha(v);
    return color;
}

QString eventType(QEvent *ev)
{
    static int eventEnumIndex = QEvent::staticMetaObject.indexOfEnumerator("Type");
    QString name = QEvent::staticMetaObject.enumerator(eventEnumIndex).valueToKey(ev->type());

    if (!name.isEmpty())
        return name;
    else
        return QVariant(ev->type()).toString();
}

void drawText(QPainter *painter, QString text, QFont font, QColor fontColor,
                           QRectF rcT, int flags, int renderingMode, int textRotate)
{
    if (renderingMode == 1) {
        QString htmlText = plainText2html(text, rcT, flags, font, fontColor, textRotate);
        QTextDocument td;
        td.setDefaultFont(painter->font());
        td.setHtml(htmlText);

        if (textRotate == 0)
            td.setTextWidth(rcT.width());
        if (textRotate == 1)
            td.setTextWidth(rcT.height());
        if (textRotate == 2)
            td.setTextWidth(rcT.width());
        if (textRotate == 3)
            td.setTextWidth(rcT.height());

        painter->save();
        painter->translate(-3,-3);
        td.drawContents(painter);
        //td.drawContents(painter, rcT.translated( -rcT.topLeft() ) );
        painter->restore();
    } else {
        if (textRotate == 0)
            painter->drawText(rcT, flags, text);
        if (textRotate == 1)
            painter->drawText(0, 0, rcT.height(), rcT.width(), flags, text);
        if (textRotate == 2)
            painter->drawText(0, 0, rcT.width(), rcT.height(), flags, text);
        if (textRotate == 3)
            painter->drawText(0, 0, rcT.height(), rcT.width(), flags, text);
    }
}

QString plainText2html(QString text, QRectF rcT, int flags, QFont font, QColor fontColor, int textRotate)
{
    QString htmlText = Qt::convertFromPlainText(text);
    htmlText.replace("&lt;sub&gt;","<sub>").replace("&lt;/sub&gt;","</sub>");
    htmlText.replace("&lt;sup&gt;","<sup>").replace("&lt;/sub&gt;","</sup>");
    htmlText.replace("&lt;b&gt;","<b>").replace("&lt;/b&gt;","</b>");
    htmlText.replace("&lt;i&gt;","<i>").replace("&lt;/i&gt;","</i>");
    htmlText.replace("&lt;u&gt;","<u>").replace("&lt;/u&gt;","</u>");
    htmlText.replace("&lt;s&gt;","<s>").replace("&lt;/s&gt;","</s>");

    QString alignParam;
    if (flags & Qt::AlignCenter)
        alignParam = "align='center'";
    if (flags & Qt::AlignLeft)
        alignParam = "align='left'";
    if (flags & Qt::AlignRight)
        alignParam = "align='right'";
    if (flags & Qt::AlignJustify)
        alignParam = "align='justify'";

    QString valignParam;
    if (flags & Qt::AlignTop)
        valignParam = valignParam + " valign='top'";
    if (flags & Qt::AlignBottom)
        valignParam = valignParam + " valign='bottom'";
    if (flags & Qt::AlignVCenter)
        valignParam = valignParam + " valign='middle'";

    int width = rcT.width();
    int height = rcT.height();
    if (textRotate == 1 || textRotate == 3) {
        width = rcT.height();
        height = rcT.width();
    }

    QString fontTag = "<table cellpadding='0' width='"+QString::number(width)+
                      "' height='"+QString::number(height)+"'><tr><td" + valignParam + ">"
                      "<span style='font-size:" + QString::number(font.pointSize() ) + "pt;" +
                      "font-family:" + font.family() + ";" +
                      "color:" + fontColor.name() + ";" +
                      "'>";


    htmlText.replace("<p>","<p " +alignParam+ ">");
    //htmlText = "<table width='200' height='500'><tr><td style='vertical-align:bottom'>dddd</td><tr></table>";

    if (font.bold())
        htmlText = "<b>" + htmlText + "</b>";
    if (font.italic())
        htmlText = "<i>" + htmlText + "</i>";
    if (font.underline())
        htmlText = "<u>" + htmlText + "</u>";

    htmlText = fontTag + htmlText + "</span></td></tr></table>";

//    qDebug() << htmlText;

    return htmlText;
}
