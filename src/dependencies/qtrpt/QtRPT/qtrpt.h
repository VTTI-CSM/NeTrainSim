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

#include <QPainter>
#include <QDomDocument>
#include <QPrintPreviewWidget>
#include <QTextDocument>
#include <qtrptnamespace.h>
#include <RptFieldObject.h>
#include <RptBandObject.h>
#include <RptPageObject.h>
#include <RptScriptEngine.h>
#include <RptSql.h>

#if QT_VERSION >= 0x50000
    #ifdef QXLSX_LIBRARY
        #include "xlsxdocument.h"
    #endif
#endif

#if QT_VERSION >= 0x50800 && QT_VERSION < 0x60000
    #include <QtCharts>
    using namespace QtCharts;
#endif

#if QT_VERSION >= 0x60000
    #include <QtCharts/QChartGlobal>
    QT_BEGIN_NAMESPACE
    class QChartView;
    class QChart;
    QT_END_NAMESPACE
#endif

using namespace QtRptName;


class RptSql;
class RptPageObject;
class RptBandObject;
class RptFieldObject;
class RptCrossTabObject;



struct DataSetInfo {
    QString dsName;
    int reportPage;
    int dataSetNo;
    int recordCount;
};
typedef QList<DataSetInfo> DataSetInfoList;


//***************************************************

//struct GraphValue {
//    QString caption;  //for Pie, for Line - ignore
//    double valueX;    //for Line only
//    double valueY;
//};

//struct GraphData {
//    QList<GraphValue> valueList;
//    QString graphDS;
//    QString caption;  //for Pie - ignore
//};
//typedef QList<GraphData> GraphDataList;

//***************************************************

typedef QList<RptPageObject*> PageList;
Q_DECLARE_METATYPE(PageList)



#ifndef QTRPT_LIBRARY
    class QtRPT : public QObject
#else
    #include <qtrpt_global.h>
    class QTRPTSHARED_EXPORT QtRPT : public QObject
#endif

{
    Q_OBJECT
    Q_PROPERTY(PageList pageList MEMBER pageList)

    friend class RptFieldObject;
    friend class RptBandObject;

public:
    typedef QSharedPointer<QtRPT> SPtrQtRPT;

	explicit QtRPT(QObject *parent = nullptr);
    static SPtrQtRPT createSPtr(QObject *parent = nullptr)
    {
        SPtrQtRPT sptr= SPtrQtRPT(new QtRPT(parent));
        return sptr;
    }

    bool setPainter(QPainter *painter);
    bool setPrinter(QPrinter *printer);
    void setResolution(QPrinter::PrinterMode resolution);
    bool loadReport(QString fileName);
    bool loadReport(QDomDocument xmlDoc);
    void clearObject();
    QDomDocument xmlDoc();
    void printExec(bool maximum = false, bool direct = false, QString printerName = QString());
    //void setCallbackFunc(void (*func)(int &recNo, QString &paramName, QVariant &paramValue));
    void setBackgroundImageOpacity(float opacity);
    void setBackgroundImage(QPixmap &image);
    void setBackgroundImage(QPixmap image);
    void printPDF(const QString &filePath, bool open = true);
    void printHTML(const QString &filePath, bool open = true);
    void printXLSX(const QString &filePath, bool open = true);
    void setSqlQuery(QString sqlString);
    static FieldType getFieldType(QDomElement e);
    static QString getFieldTypeName(FieldType type);
    static QSet<FieldType> getDrawingFields();
    static Qt::PenStyle getPenStyle(QString value);
    PageList pageList;

    ~QtRPT();
    Q_INVOKABLE RptPageObject *getPage(int pageNo);
    static QString getFormattedValue(QString value, QString formatString, QString inputFormatString);

    void setUserSqlConnection(int pageReportNo, QString dsName, QString dbType, QString dbName, QString dbHost, QString dbUser, QString dbPassword, int dbPort, QString dbConnectionName, QString sql, QString dbCoding = "UTF8", QString charsetCoding = "UTF8");
    void activateUserSqlConnection(int pageReportNo, bool bActive);

protected:
    bool eventFilter(QObject *obj, QEvent *e);

private:
    QPixmap *m_backgroundImage;
    QPrinter *printer;
    QPainter *painter;
    int m_recNo;
    int mg_recNo;
    int m_pageReport;
    double koefRes_h;
    double koefRes_w;
    int ph;
    int pw;
    int ml;
    int mr;
    int mt;
    int mb;
    int currentPage;
    int totalPage;
    int m_orientation;
    float m_backgroundOpacity;
    RptSqlConnection m_userSqlConnection;
    QString m_globalScript;
    RptScriptEngine *m_globalEngine;
    QPrintPreviewWidget *pr;
    QList<QAction*> lst;
    QDomDocument m_xmlDoc;
    QDomNode getBand(BandType type, QDomElement docElem);
    void drawBandRow(RptBandObject *band, int bandTop, bool allowDraw = true);
    void fillListOfValue(RptBandObject *bandObject);
    QString sectionField(RptBandObject *band, QString value, bool isReachText, bool firstPass = false, QString formatString = "", QString inputFormatString = "");
    void processGlobalScript();
    QStringList splitStringOnVariable(QString strValue);
    QString getVariableValue(QString scriptStr, bool exp = false);
    QString stringPreprocessing(QString str, QString formatString);
    QImage sectionFieldImage(QString value);
    QVariant getInternalVariable(QString value, QString formatString);
    QString sectionValue(QString paramName, int recNo = -1);
    QImage sectionValueImage(QString paramName);
    void newPage(QPrinter *printer, int &y, bool draw, bool newReportPage = false);
    void processPHeader(int &y, bool draw);
    void processPFooter(bool draw);
    void processMFooter(QPrinter *printer, int &y, int dsNo, bool draw, int &pageReportNo);
    void processRSummary(QPrinter *printer, int &y, bool draw, int &pageReportNo);
    //void (*callbackFunc)(int &recNo, QString &paramName, QVariant &paramValue);
    void processReport(QPrinter *printer, bool draw, int &pageReportNo, bool isFirstPage);
    void processRTitle(int &y, bool draw);
    void processMHeader(int &y, int dsNo, bool draw, int &pageReportNo);
    void processMasterData(QPrinter *printer, int &y, bool draw, int &pageReportNo, int dsNo, QList<int> GroupIdxList_current);
    void processGroupHeader(QPrinter *printer, int &y, bool draw, int &pageReportNo);
    void setPageSettings(QPrinter *printer, int pageReportNo);
    void drawBackground(bool draw);
    bool isFieldVisible(RptFieldObject *fieldObject);
    QVariant processHighligthing(RptFieldObject *field, HiType type);
    bool allowPrintPage(bool draw, int curPage_);
    bool allowNewPage(bool draw, int curPage_);
    int fromPage;
    int toPage;
    void setFont(RptFieldObject *fieldObject);
    static Qt::Alignment getAligment(QDomElement e);
    QPen getPen(RptFieldObject *fieldObject);
    void drawFields(RptFieldObject *fieldObject, int bandTop, bool firstPass);
    void drawLines(RptFieldObject *fieldObject, int bandTop);
    void openDataSource(int &pageReportNo);
    int getRecCount(int reportPage, int dsSetNo);
    int getRecCount(int reportPage, QString dsSetName);
    bool setRecCount(int reportPage, int dsSetNo, int recCount);
    bool setRecCount(int reportPage, QString dsSetName, int recCount);
    void setUserSqlConnection(int &pageReportNo, const RptSqlConnection &sqlConnection);
    void getUserSqlConnection(int &pageReportNo, RptSqlConnection &sqlConnection);
    QString findAliasinTHML(QTextDocument *document);
    void replaceinHTML(QTextDocument *document, QString alias, QString value);

    DataSetInfoList m_dataSetInfoList;

    QString m_HTML;
    RptCrossTabObject *crossTab;

    #ifdef QXLSX_LIBRARY
        QXlsx::Document *m_xlsx;
    #endif

    void makeReportObjectStructure();
    enum PrintMode
    {
        Printer = 0,
        Pdf     = 1,
        Html    = 2,
        Xlsx    = 3
    };
    PrintMode m_printMode;
    QPrinter::PrinterMode m_resolution;

signals:
    void setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage);
    void setField(RptFieldObject &fieldObject);
    void setValueImage(const int recNo, const QString paramName, QImage &paramValue, const int reportPage);
    void setValueDiagram(GraphDataList &data);
    #if QT_VERSION >= 0x50800
        void setChart(RptFieldObject &fieldObject, QChart &chart);
    #endif
    void newPage(int page);
    void setDSInfo(DataSetInfo &dsInfo);
    void previewDestroyed();

public slots:
    void printPreview(QPrinter *printer);

private slots:
    void exportTo();

};


#ifdef QTRPT_LIBRARY
    extern "C" QTRPTSHARED_EXPORT QtRPT* createQtRPT();
#endif
