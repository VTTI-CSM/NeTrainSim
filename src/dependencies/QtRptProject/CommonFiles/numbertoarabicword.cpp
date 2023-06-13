#include "numbertoarabicword.h"
#include <qmath.h>
#include <QDebug>

NumberToArabicWord::NumberToArabicWord(double number)
{

    this->number = number;


    ExtractIntegerAndDecimalParts();
    arabicOnes = QStringList() << QString()
    << "واحد"
    << "اثنان"
    << "ثلاثة"
    << "أربعة"
    << "خمسة"
    << "ستة"
    << "سبعة"
    << "ثمانية"
    << "تسعة"
    << "عشرة"
    << "أحد عشر"
    << "اثنا عشر"
    << "ثلاثة عشر"
    << "أربعة عشر"
    << "خمسة عشر"
    << "ستة عشر"
    << "سبعة عشر"
    << "ثمانية عشر"
    << "تسعة عشر";

    arabicFeminineOnes <<  QStringList() << QString()
    << "إحدى"
    << "اثنتان"
    << "ثلاث"
    << "أربع"
    << "خمس"
    << "ست"
    << "سبع"
    << "ثمان"
    << "تسع"
    << "عشر"
    << "إحدى عشرة"
    << "اثنتا عشرة"
    << "ثلاث عشرة"
    << "أربع عشرة"
    << "خمس عشرة"
    << "ست عشرة"
    << "سبع عشرة"
    << "ثماني عشرة"
    << "تسع عشرة";

    arabicTens << "عشرون"
    << "ثلاثون"
    << "أربعون"
    << "خمسون"
    << "ستون"
    << "سبعون"
    << "ثمانون"
    << "تسعون";

    arabicHundreds << ""
    << "مائة"
    << "مئتان"
    << "ثلاثمائة"
    << "أربعمائة"
    << "خمسمائة"
    << "ستمائة"
    << "سبعمائة"
    << "ثمانمائة"
    << "تسعمائة";

    arabicTwos << "مئتان"
    << "ألفان"
    << "مليونان"
    << "ملياران"
    << "تريليونان"
    << "كوادريليونان"
    << "كوينتليونان"
    << "سكستيليونان";

    arabicAppendedTwos << "مئتان"
    << "ألفا"
    << "مليونا"
    << "مليارا"
    << "تريليونا"
    << "كوادريليونا"
    << "كوينتليونا"
    << "سكستيليونا";

    arabicGroup << "مائة"
    << "ألف"
    << "مليون"
    << "مليار"
    << "تريليون"
    << "كوادريليون"
    << "كوينتليون"
    << "سكستيليون";

    arabicAppendedGroup << QString()
    << "ألفاً"
    << "مليوناً"
    << "ملياراً"
    << "تريليوناً"
    << "كوادريليوناً"
    << "كوينتليوناً"
    << "سكستيليوناً";

    arabicPluralGroups << QString()
    << "آلاف"
    << "ملايين"
    << "مليارات"
    << "تريليونات"
    << "كوادريليونات"
    << "كوينتليونات"
    << "سكستيليونات";
}

QString NumberToArabicWord::ConvertToArabic()
{
    if (number == 0)
    {
        return "صفر";
    }
    else if(number==1)
    {
        return "واحد";
    }
    else if(number==2)
    {
        return "اثنان";
    }

    double tempNumber = number;

    // Get Text for the decimal part

    QString decimalString = ProcessArabicGroup(_decimalValue, -1, 0);


    QString retVal = QString();
    short group = 0;

    while (tempNumber >= 1)
    {
        // seperate number into groups
        int numberToProcess = (int)(tempNumber) % 1000;

        tempNumber = tempNumber / 1000;

        // convert group into its text

        QString groupDescription = ProcessArabicGroup(numberToProcess, group, qFloor(tempNumber));

        if (!groupDescription.isEmpty())
        { // here we add the new converted group to the previous concatenated text
            if (group > 0)
            {
                if (!retVal.isEmpty())
                    retVal = QString("%1 %2").arg("و", retVal);

                if (numberToProcess != 2)
                {
                    if (numberToProcess % 100 != 1)
                    {
                        if (numberToProcess >= 3 && numberToProcess <= 10) // for numbers between 3 and 9 we use plural name
                            retVal = QString("%1 %2").arg(arabicPluralGroups[group], retVal);
                        else
                        {
                            if (!retVal.isEmpty()) // use appending case
                                retVal = QString("%1 %2").arg(arabicAppendedGroup[group], retVal);
                            else
                                retVal = QString("%1 %2").arg(arabicGroup[group], retVal); // use normal case
                        }
                    }
                }
            }

            retVal = QString("%1 %2").arg(groupDescription, retVal);
        }

        group++;
    }

    QString formattedNumber = QString();

    formattedNumber += (!retVal.isEmpty()) ? retVal : QString();
    formattedNumber += (_decimalValue != 0) ? " و " : "";
    formattedNumber += (_decimalValue != 0) ? decimalString : "";



    return formattedNumber;
}

QString NumberToArabicWord::ProcessArabicGroup(int groupNumber, int groupLevel, qulonglong remainingNumber)
{
    int tens = groupNumber % 100;

    int hundreds = groupNumber / 100;

    QString retVal = "";

    if (hundreds > 0)
    {
        if (tens == 0 && hundreds == 2) // حالة المضاف
            retVal = QString("%1").arg(arabicAppendedTwos[0]);
        else //  الحالة العادية
            retVal = QString("%1").arg(arabicHundreds[hundreds]);
    }

    if (tens > 0)
    {
        if (tens < 20)
        { // if we are processing under 20 numbers
            if (tens == 2 && hundreds == 0 && groupLevel > 0)
            { // This is special case for number 2 when it comes alone in the group
                if (_intergerValue == 2000 || _intergerValue == 2000000 || _intergerValue == 2000000000 || _intergerValue == 2000000000000 || _intergerValue == 2000000000000000 || _intergerValue == 2000000000000000000)
                    retVal = QString("%1").arg(arabicAppendedTwos[groupLevel]); // في حالة الاضافة
                else
                    retVal = QString("%1").arg(arabicTwos[groupLevel]);//  في حالة الافراد
            }
            else
            { // General case
                if (!retVal.isEmpty())
                    retVal += " و ";

                if (tens == 1 && groupLevel > 0)
                    retVal += arabicGroup[groupLevel];
                else
                    if ((tens == 1 || tens == 2) && (groupLevel == 0 || groupLevel == -1) && hundreds == 0 && remainingNumber == 0)
                        retVal += QString(); // Special case for 1 and 2 numbers like: ليرة سورية و ليرتان سوريتان
                    else
                        retVal += GetDigitFeminineStatus(tens, groupLevel);// Get Feminine status for this digit
            }
        }
        else
        {
            int ones = tens % 10;
            tens = (tens / 10) - 2; // 20's offset

            if (ones > 0)
            {
                if (!retVal.isEmpty())
                    retVal += " و ";

                // Get Feminine status for this digit
                retVal += GetDigitFeminineStatus(ones, groupLevel);
            }

            if (!retVal.isEmpty())
                retVal += " و ";

            // Get Tens text
            retVal += arabicTens[tens];
        }
    }

    return retVal;
}

QString NumberToArabicWord::GetDigitFeminineStatus(int digit, int groupLevel)
{
    if (groupLevel == -1)  // if it is in the decimal part
        return arabicOnes.at(digit);
    else if (groupLevel == 0)
        return arabicOnes.at(digit);
    else
        return arabicOnes.at(digit);
}

void NumberToArabicWord::ExtractIntegerAndDecimalParts()
{
    QStringList splits = QString::number(number).split('.');

    bool ok;
    _intergerValue = splits.at(0).toULongLong(&ok);

    if(!ok) _intergerValue = 0;

    ok = false; // assuming there is no decimal value by default
    if (splits.count() > 1)
        _decimalValue = GetDecimalValue(splits.at(1)).toULongLong(&ok);

    if(!ok) _decimalValue = 0;
}

QString NumberToArabicWord::GetDecimalValue(QString decimalPart)
{
    QString result = "";
    result = decimalPart;

    return result;
}
