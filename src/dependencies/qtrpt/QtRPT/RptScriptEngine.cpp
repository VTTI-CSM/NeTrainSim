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

#include "RptScriptEngine.h"
#include <QDebug>
#include <QtMath>
#include "CommonClasses.h"
#include "qtrpt.h"

Q_DECLARE_METATYPE(QColor);

RptScriptEngine::RptScriptEngine(QObject *parent)
: QJSEngine(parent)
{
    qRegisterMetaType<PageList>("<QList<RptPageObject*> >");
    qRegisterMetaType<PageList>("PageList");
    qRegisterMetaType<QtRPT*>("QtRPT*");
    qRegisterMetaType<RptPageObject*>("RptPageObject*");
    qRegisterMetaType<RptBandObject*>();
    qRegisterMetaType<RptFieldObject*>();

    scriptAPI.setParent(this);
    QJSValue wrapobj = this->newQObject(&scriptAPI);
    this->globalObject().setProperty("QColor", wrapobj.property("qcolorValue"));
    this->globalObject().setProperty("Replace", wrapobj.property("funcReplace"));
    this->globalObject().setProperty("ToUpper", wrapobj.property("funcToUpper"));
    this->globalObject().setProperty("ToLower", wrapobj.property("funcToLower"));
    this->globalObject().setProperty("Frac", wrapobj.property("funcFrac"));
    this->globalObject().setProperty("Floor", wrapobj.property("funcFloor"));
    this->globalObject().setProperty("Ceil", wrapobj.property("funcCeil"));
    this->globalObject().setProperty("Round", wrapobj.property("funcRound"));
    this->globalObject().setProperty("debug", wrapobj.property("funcDebug"));
    this->globalObject().setProperty("NumberToWords", wrapobj.property("funcNumberToWords"));
    this->globalObject().setProperty("SUM", wrapobj.property("funcAggregate"));

    addObject(parent);
}

void RptScriptEngine::addObject(QObject *object)
{
    QJSValue scriptObject = this->newQObject(object);

    auto qtrpt = qobject_cast<QtRPT*>(object);
    if (qtrpt)
        this->globalObject().setProperty("QtRPT", scriptObject);
    else
        this->globalObject().setProperty(object->objectName(), scriptObject);
}

QJSValue RptScriptEngine::evaluate(const QString &program, const QString &fileName, int lineNumber)
{
    Q_UNUSED(fileName);
    Q_UNUSED(lineNumber);

    QJSValue result = QJSEngine::evaluate(program);

    if (result.isError())
    {
            qDebug()
                    << "Uncaught exception at line"
                    << result.property("lineNumber").toInt()
                    << ":" << result.toString();
    }

    return result;
}


//--------------------------------------------------------

ScriptAPI::ScriptAPI(QObject *parent)
: QObject(parent)
{

}

QColor ScriptAPI::qcolorValue(int r, int g, int b)
{
    return QColor(r,g,b,255);
}

QString ScriptAPI::funcReplace(QString param, QString strBefore, QString strAfter)
{
    return param.replace(strBefore, strAfter);
}

QString ScriptAPI::funcToUpper(QString param)
{
    return param.toUpper();
}

QString ScriptAPI::funcToLower(QString param)
{
    return param.toLower();
}

int ScriptAPI::funcFrac(double value)
{
    int b = qFloor(value);
    b = (value-b) * 100 + 0.5;
    return b;
}

int ScriptAPI::funcFloor(double value)
{
    return qFloor(value);
}

int ScriptAPI::funcCeil(double value)
{
    return qCeil(value);
}

int ScriptAPI::funcRound(double value)
{
    return qRound(value);
}

void ScriptAPI::funcDebug(QString value)
{
    qDebug() << value;
}

QString ScriptAPI::funcNumberToWords(QString paramLanguage, double value)
{
    return double2Money(value, paramLanguage);
}


double ScriptAPI::funcAggregate(int funcMode, QString paramName)
{
    paramName = paramName.replace("'", "");
    auto engine = qobject_cast<RptScriptEngine*>(this->parent());

    double total = 0;
    double min = 0;
    double max = 0;
    int count = 0;

    QList<int> *groupIdxList = nullptr;
    if (engine->globalObject().property("groupLevel").toInt() == 0)
        groupIdxList = &GroupIdxList_0;
    else if (engine->globalObject().property("groupLevel").toInt() == 1)
        groupIdxList = &GroupIdxList_1;
    else if (engine->globalObject().property("groupLevel").toInt() == 2)
        groupIdxList = &GroupIdxList_2;

    if (!listOfPair.isEmpty())
        min = listOfPair.first().paramValue.toDouble();  //set initial value for Min

    for (auto &aggValues : listOfPair) {
        if (aggValues.paramName == paramName) {
            if (groupIdxList && !groupIdxList->isEmpty() && engine->globalObject().property("showInGroup").toBool() == true) {
                for (auto &grpIdx : *groupIdxList)
                {
                    if (grpIdx == aggValues.lnNo)
                    {
                        total += aggValues.paramValue.toDouble();
                        count += 1;
                        if (max < aggValues.paramValue.toDouble())
                            max = aggValues.paramValue.toDouble();
                        if (min > aggValues.paramValue.toDouble())
                            min = aggValues.paramValue.toDouble();
                    }
                }
            } else {
                if (!aggValues.paramValue.toString().isEmpty()) {
                    total += aggValues.paramValue.toDouble();
                    count += 1;
                    if (max < aggValues.paramValue.toDouble())
                        max = aggValues.paramValue.toDouble();
                    if (min > aggValues.paramValue.toDouble())
                        min = aggValues.paramValue.toDouble();
                }
            }
        }
    }

    switch (funcMode) {
    case 0:  //SUM
        return total;
    case 1:  //AVG
        if (count > 0)
            return total/count;
        else
            return 0;
    case 2:  //COUNT
        return count;
    case 3:  //MAX
        return max;
    case 4:  //MIN
        return min;
    default: return 0;
    }
    return 0;
}

