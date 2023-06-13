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

#pragma once

#include <QJSEngine>
#include <QColor>

struct AggregateValues
{
    QString paramName;
    QVariant paramValue;
    int lnNo;
    int pageReport;
};

extern QList<AggregateValues> listOfPair;
extern QList<int> GroupIdxList_0;
extern QList<int> GroupIdxList_1;
extern QList<int> GroupIdxList_2;

struct RowData {
    int ln_no;
    QString lvl0_data;
    QString lvl1_data;
    QString lvl2_data;
};

extern QList<RowData> rowList;

//--------------------------------------------------------

class ScriptAPI : public QObject
{
    Q_OBJECT
public:
    explicit ScriptAPI(QObject *parent = nullptr);
    Q_INVOKABLE double funcAggregate(int funcMode, QString paramName);
    Q_INVOKABLE QColor qcolorValue(int r, int g, int b);
    Q_INVOKABLE QString funcReplace(QString param, QString strBefore, QString strAfter);
    Q_INVOKABLE QString funcToUpper(QString param);
    Q_INVOKABLE QString funcToLower(QString param);
    Q_INVOKABLE int funcFrac(double value);
    Q_INVOKABLE int funcFloor(double value);
    Q_INVOKABLE int funcCeil(double value);
    Q_INVOKABLE int funcRound(double value);
    Q_INVOKABLE void funcDebug(QString value);
    Q_INVOKABLE QString funcNumberToWords(QString paramLanguage, double value);
};

//--------------------------------------------------------

class RptScriptEngine : public  QJSEngine
{
    Q_OBJECT
public:
    RptScriptEngine(QObject *parent = nullptr);
    void addObject(QObject *object);
    QJSValue evaluate(const QString &program, const QString &fileName = QString(), int lineNumber = 1);

private:
    ScriptAPI scriptAPI;

};
