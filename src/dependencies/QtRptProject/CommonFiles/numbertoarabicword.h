#pragma once

#include <QObject>
#include <QStringList>


class NumberToArabicWord : public QObject
{
public:

    NumberToArabicWord(double number);
    QString ConvertToArabic();

    double number;


private:

    QStringList arabicOnes;

    QStringList arabicFeminineOnes;

    QStringList arabicTens;

    QStringList arabicHundreds;

    QStringList arabicTwos;

    QStringList arabicAppendedTwos;

    QStringList arabicGroup;

    QStringList arabicAppendedGroup;

    QStringList arabicPluralGroups;

    QString ProcessArabicGroup(int groupNumber, int groupLevel, qulonglong remainingNumber);

    QString GetDigitFeminineStatus(int digit, int groupLevel);

    quint64 _intergerValue;


    int _decimalValue;

    void ExtractIntegerAndDecimalParts();

    QString GetDecimalValue(QString decimalPart);


};

