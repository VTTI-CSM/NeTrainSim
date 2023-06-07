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

#include "qtrpt.h"
#include <QBuffer>
#include <QApplication>
#include <QAction>
#include <QTime>
#include <QFile>
#include <QPrintPreviewDialog>
#include <QFileDialog>
#include <QDesktopServices>
#include <QPrinterInfo>
#include <QTextCursor>
#include <QTextBlock>
#include <QPrintDialog>
#include <QRegularExpression>
#include <QUrl>
#include <QToolBar>
#include "CommonClasses.h"
#include "RptSql.h"
#include "RptDsInline.h"
#include "RptDsPlugin.h"
#include "Barcode.h"

QList<AggregateValues> listOfPair;
QList<int> GroupIdxList_0;
QList<int> GroupIdxList_1;
QList<int> GroupIdxList_2;
QList<RowData> rowList;

/*!
 \namespace QtRptName
 \inmodule qtrptnamespace
 \brief Contains miscellaneous identifiers used throughout the QtRPT library.
*/

/*!
 \inmodule qtrptnamespace
 \enum QtRptName::BandType

 This enum describes the type of band.

 \value ReportTitle
 \value PageHeader
 \value DataGroupHeader
 \value MasterHeader
 \value MasterData
 \value MasterFooter
 \value DataGroupFooter
 \value ReportSummary
 \value PageFooter
*/

/*!
 \inmodule qtrptnamespace
 \enum QtRptName::FieldType

 This enum describes the type of Field.

 \value Text
 \value TextImage
 \value TextRich
 \value Image
 \value Diagram
 \value Reactangle
 \value RoundedReactangle
 \value Circle
 \value Triangle
 \value Rhombus
 \value Line
 \value Barcode
*/

/*!
 \inmodule qtrptnamespace
 \enum QtRptName::BorderStyle

 This enum describes the border's style.

 \value Dashed
 \value Dot_dash
 \value Dot_dot_dash
 \value Dotted
 \value Double
 \value Groove
 \value Inset
 \value Outset
 \value Ridge
 \value Solid
 \value BorderNone
*/

/*!
 \inmodule qtrptnamespace
 \enum QtRptName::Command

 This enum describes the command, used in QtRptDesigner.
 \value None
 \value Name
 \value Bold
 \value Italic
 \value Underline
 \value Strikeout
 \value Font
 \value FontSize
 \value FontColor
 \value FontName
 \value Frame
 \value FrameLeft
 \value FrameRight
 \value FrameTop
 \value FrameBottom
 \value FrameNo
 \value FrameAll
 \value FrameStyle
 \value FrameWidth
 \value AligmentH
 \value AligmentV
 \value Left
 \value Top
 \value Width
 \value Height
 \value Length
 \value BackgroundColor
 \value BorderColor
 \value Printing
 \value StartNewNumeration
 \value ShowInGroup
 \value StartNewPage
 \value AutoHeight
 \value ArrowStart
 \value ArrowEnd
 \value IgnoreRatioAspect
 \value BarcodeType
 \value BarcodeFrameType
 \value TextWrap
*/

QtRPT* createQtRPT()
{
     auto z = new QtRPT();
     return z;
}

/*!
 \class QtRPT
 \inmodule qtrpt
 \brief The QtRPT class is the base class of the QtRPT.

  QtRPT is the easy-to-use print report engine written in C++ QtToolkit.
  It allows combining several reports in one XML file.

  For separately taken field, you can specify some condition depending on which this
  field will display in different font and background color, etc.
*/

/*!
  \fn QtRPT::QtRPT(QObject *parent)
  Constructs a QtRPT object with the given \a parent.
*/

/*!
  \since 2.0.1

  \fn static SPtrQtRPT QtRPT::createSPtr(QObject *parent)
  Constructs a QtRPT object as a QSharedPointer with the given \a parent.
  The SPtrQtRPT is : \c {typedef QSharedPointer<QtRPT> SPtrQtRPT;}
 */
QtRPT::QtRPT(QObject *parent)
: QObject(parent)
{
    qRegisterMetaType<DataSetInfo>("DataSetInfo");

    m_globalEngine = new RptScriptEngine(this);
    m_xmlDoc = QDomDocument("Reports");
    m_backgroundImage = nullptr;
    m_orientation = 0;
    m_backgroundOpacity = 1;
    m_userSqlConnection.active = false;
    m_printMode = QtRPT::Printer;
    m_resolution = QPrinter::HighResolution;
    //m_resolution = QPrinter::ScreenResolution;
    painter = nullptr;
    printer = nullptr;
    crossTab = nullptr;
    #ifdef QXLSX_LIBRARY
        m_xlsx = nullptr;
    #endif
}

/*!
  \fn QtRPT::setResolution(QPrinter::PrinterMode resolution)
  Sets \a resolution of the printer
*/
void QtRPT::setResolution(QPrinter::PrinterMode resolution)
{
    m_resolution = resolution;
}

/*!
 \fn QtRPT::loadReport(QString fileName)
  Loads report from XML file with \a fileName.
 Returns \c true if loading is success
 */
bool QtRPT::loadReport(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    } else {
        listOfPair.clear();
        GroupIdxList_0.clear();
        GroupIdxList_1.clear();
        GroupIdxList_2.clear();
    }

    if (!m_xmlDoc.setContent(&file)) {
        file.close();
        qWarning() << "Report file not found";
        return false;
    }

    file.close();
    makeReportObjectStructure();

    return true;
}

/*! \overload
 Loads report from QDomDocument \a xmlDoc.
 Returns \c true if loading is success
*/
bool QtRPT::loadReport(QDomDocument xmlDoc)
{
    QtRPT::m_xmlDoc = xmlDoc;

    listOfPair.clear();
    GroupIdxList_0.clear();
    GroupIdxList_1.clear();
    GroupIdxList_2.clear();
    makeReportObjectStructure();
    return true;
}

/*!
 \fn QtRPT::xmlDoc()
  Returns \c QDomDocument of the report.
 */
QDomDocument QtRPT::xmlDoc()
{
    return m_xmlDoc;
}

/*!
 \fn QtRPT::setPainter(QPainter *painter)
  Set the \a painter that will be used for the report to draw.
  Returns \c true if assignment is success
 */
bool QtRPT::setPainter(QPainter *painter)
{
    if (this->painter == nullptr) {
        this->painter = painter;
        return true;
    }
    return false;
}

/*!
 \fn QtRPT::setPrinter(QPrinter *printer)
  Set the \a printer that will be used for the report to printing.
  Returns \c true if assignment is success
 */
bool QtRPT::setPrinter(QPrinter *printer)
{
    if (this->printer == nullptr) {
        this->printer = printer;
        return true;
    };
    return false;
}

void QtRPT::makeReportObjectStructure()
{
    clearObject();
    for (int i = 0; i < m_xmlDoc.documentElement().childNodes().count(); i++) {
        QDomNode domNode = m_xmlDoc.documentElement().childNodes().at(i);
        QDomElement docElem = domNode.toElement();

        if (docElem.tagName() == "Report") {
            auto pageObject = new RptPageObject(this);
            pageObject->setProperty(this, docElem);
            pageList.append(pageObject);
        } else if (docElem.tagName() == "Script") {
            QDomNode cdataNode = domNode.childNodes().at(0);
            if (cdataNode.isCDATASection()) {
                m_globalScript = cdataNode.toCDATASection().data();
            }
        }
    }
}

/*!
 \fn QtRPT::~QtRPT()
  Destroys the object, deleting all its child objects.
 */
QtRPT::~QtRPT()
{
    clearObject();
}

/*!
 \fn QtRPT::clearObject()
 Destroy all objects and clear the report.
 */
void QtRPT::clearObject()
{
    qDeleteAll(pageList);
    pageList.clear();
}

int QtRPT::getRecCount(int reportPage, int dsSetNo)
{
    int count = 0;
    for (const auto &ds : m_dataSetInfoList)
        if (ds.reportPage == reportPage && ds.dataSetNo == dsSetNo)
            count = ds.recordCount;

    return count;
}

int QtRPT::getRecCount(int reportPage, QString dsSetName)
{
    int count = 0;
    for (const auto &ds : m_dataSetInfoList)
        if (ds.reportPage == reportPage && ds.dsName == dsSetName)
            count = ds.recordCount;

    return count;
}

bool QtRPT::setRecCount(int reportPage, int dsSetNo, int recCount)
{
    bool found = false;
    for (auto &ds : m_dataSetInfoList)
        if (ds.reportPage == reportPage && ds.dataSetNo == dsSetNo) {
            ds.recordCount = recCount;
            found = true;
        }

    return found;
}

bool QtRPT::setRecCount(int reportPage, QString dsSetName, int recCount)
{
    bool found = false;
    for (auto &ds : m_dataSetInfoList)
        if (ds.reportPage == reportPage && ds.dsName == dsSetName) {
            ds.recordCount = recCount;
            found = true;
        }

    return found;
}

QDomNode QtRPT::getBand(BandType type, QDomElement docElem)
{
    QString s_type;
    if (type == ReportTitle)     s_type = "ReportTitle";
    if (type == PageHeader)      s_type = "PageHeader";
    if (type == MasterData)      s_type = "MasterData";
    if (type == PageFooter)      s_type = "PageFooter";
    if (type == ReportSummary)   s_type = "ReportSummary";
    if (type == MasterFooter)    s_type = "MasterFooter";
    if (type == MasterHeader)    s_type = "MasterHeader";
    if (type == DataGroupHeader) s_type = "DataGroupHeader";
    if (type == DataGroupFooter) s_type = "DataGroupFooter";

    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if (!e.isNull() && e.tagName() == "ReportBand")
            if (e.attribute("type") == s_type)
                return n;

        n = n.nextSibling();
    }
    return n;
}

void QtRPT::setFont(RptFieldObject *fieldObject)
{
    if (painter->isActive()) {
        painter->setFont(fieldObject->font);
        painter->setPen(Qt::black);
    }
}

Qt::Alignment QtRPT::getAligment(QDomElement e)
{
    Qt::Alignment al;
    Qt::Alignment alH, alV;
    if (e.attribute("aligmentH") == "hRight")   alH = Qt::AlignRight;
    if (e.attribute("aligmentH") == "hLeft")    alH = Qt::AlignLeft;
    if (e.attribute("aligmentH") == "hCenter")  alH = Qt::AlignHCenter;
    if (e.attribute("aligmentH") == "hJustify") alH = Qt::AlignJustify;
    if (e.attribute("aligmentV") == "vTop")     alV = Qt::AlignTop;
    if (e.attribute("aligmentV") == "vBottom")  alV = Qt::AlignBottom;
    if (e.attribute("aligmentV") == "vCenter")  alV = Qt::AlignVCenter;
    return al = alH | alV;
}

QPen QtRPT::getPen(RptFieldObject *fieldObject)
{
    QPen pen;
    if (painter->isActive())
        pen = painter->pen();

    //Set border width
    int width;
    switch (m_resolution) {
    case QPrinter::ScreenResolution:
        width = 1;
        break;
    case QPrinter::HighResolution:
    default:
        width = 5;
        break;
    }
    pen.setWidth(fieldObject->borderWidth * width);

    //Set border style
    QString borderStyle = fieldObject->borderStyle;
    pen.setStyle(getPenStyle(borderStyle));
    return pen;
}

/*!
 \fn Qt::PenStyle QtRPT::getPenStyle(QString value)
 Convert and return Pen style of field for given \a value
 */
Qt::PenStyle QtRPT::getPenStyle(QString value)
{
    if (value == "dashed")            return Qt::DashLine;
    else if (value == "dotted")       return Qt::DotLine;
    else if (value == "dot-dash")     return Qt::DashDotLine;
    else if (value == "dot-dot-dash") return Qt::DashDotDotLine;
    else                              return Qt::SolidLine;
}

/*!
 \fn QtRPT::getFieldType(QDomElement e)
 Return field's type of given QDomElement \a e which represents a field
 \sa getFieldTypeName()
 */
FieldType QtRPT::getFieldType(QDomElement e)
{
    if (e.attribute("type","label") == "barcode") return Barcode;
    else if (e.attribute("type","label") == "reactangle") return Reactangle;
    else if (e.attribute("type","label") == "roundedReactangle") return RoundedReactangle;
    else if (e.attribute("type","label") == "circle") return Circle;
    else if (e.attribute("type","label") == "triangle") return Triangle;
    else if (e.attribute("type","label") == "rhombus") return Rhombus;
    else if (e.attribute("type","label") == "textRich") return TextRich;
    else if (e.attribute("type","label") == "label") return Text;
    else if (e.attribute("type","label") == "labelImage") return TextImage;
    else if (e.attribute("type","label") == "image" || e.attribute("picture","text") != "text") return Image;
    else if (e.attribute("type","label") == "diagram") return Diagram;
    else if (e.attribute("type","label") == "line") return Line;
    else if (e.attribute("type","label") == "DatabaseImage") return DatabaseImage;
    else if (e.attribute("type","label") == "crossTab") return CrossTab;
    else return Text;
}

/*!
 \fn QString QtRPT::getFieldTypeName(FieldType type)
 Return the field's type name for given \a type
 \sa getFieldType()
 */
QString QtRPT::getFieldTypeName(FieldType type)
{
    switch (type) {
        case Reactangle: return "reactangle";
        case RoundedReactangle: return "roundedReactangle";
        case Circle: return "circle";
        case Triangle: return "triangle";
        case Rhombus: return "rhombus";
        case TextRich: return "textRich";
        case Text: return "label";
        case TextImage: return "labelImage";
        case Image: return "image";
        case Diagram: return "diagram";
        case Line: return "line";
        case Barcode: return "barcode";
        case DatabaseImage: return "DatabaseImage";
        case CrossTab: return "crossTab";
        default: return "label";
    }
}

/*!
 \fn QList<FieldType> QtRPT::getDrawingFields()
 Return the QList contains the type of fields which acts as a drawing fields.
 The following fields are a drawing:
 \list
    \li Circle
    \li Triangle
    \li Rhombus
    \li RoundedReactangle
    \li Reactangle
 \endlist
 */
QSet<FieldType> QtRPT::getDrawingFields()
{
    QSet<FieldType> set;
    set << Circle << Triangle << Rhombus << RoundedReactangle << Reactangle;
    return set;
}

void QtRPT::drawFields(RptFieldObject *fieldObject, int bandTop, bool draw)
{
    fieldObject->value = fieldObject->value.replace("&Acirc","");
    fieldObject->m_recNo = m_recNo;
    fieldObject->m_reportPage = m_pageReport;
    if (draw)
        fieldObject->updateHighlightingParam();

    // we request data if it is a not child of the CrossTab
    if (!fieldObject->isCrossTabChild())
        emit setField(*fieldObject);

    if (!isFieldVisible(fieldObject))
        return;

    if (fieldObject->isCrossTabChild()) {
        bool isTotalField = fieldObject->parentCrossTab->isTotalField(fieldObject);
        bool isHeaderField = fieldObject->parentCrossTab->isHeaderField(fieldObject);

        if (isTotalField == false && isHeaderField == false)
            // we request data if it is a child of the CrossTab,
            //but it is a not Total field
            emit setField(*fieldObject);
        else if (isTotalField == true)
            // if it is a Total field, we a calculate Total
            fieldObject->parentCrossTab->total(fieldObject);
    }

    // Process GlobalScript for current field --START--
    if (draw) {
        QJSValue beforeData = m_globalEngine->globalObject().property(fieldObject->objectName() + "BeforeData");
        if (!beforeData.isNull())
            beforeData.call(QJSValueList());

        if (!fieldObject->isVisible())
            return;
    }
    // Process GlobalScript for current field --END--


    int left_   = fieldObject->rect.x() * koefRes_w;
    //int width_  = fieldObject->rect.width() * koefRes_w;
    int width_  = (fieldObject->rect.width()-1) * koefRes_w;
    int height_ = fieldObject->rect.height() * koefRes_h;
    int top_    = (bandTop+fieldObject->rect.y()) * koefRes_h;

    fieldObject->setHTMLTop(top_/koefRes_h);

    if (fieldObject->autoHeight == 1)
        if (fieldObject->parentBand != nullptr)
            height_ = fieldObject->parentBand->realHeight * koefRes_h;

    FieldType fieldType = fieldObject->fieldType;
    QPen pen = getPen(fieldObject);



    // Drawing Border, Background, Not Text fields --START--
    if (draw) {
        if (!getDrawingFields().contains(fieldType)
            && fieldType != Barcode
            && fieldType != Image
            && fieldType != CrossTab
            && fieldType != Diagram
        ) {
            // Fill background
            if ( fieldObject->backgroundColor  != QColor(255,255,255,255)) {
                if (painter->isActive())
                    painter->fillRect(left_+1, top_+1, width_-2, height_-2, fieldObject->backgroundColor);
            }
            // Draw frame
            if (fieldObject->borderTop != QColor(255,255,255,255) && fieldObject->borderTop != QColor(255,255,255,0)) {
                pen.setColor(fieldObject->borderColor);
                if (painter->isActive()) {
                    painter->setPen(pen);
                    painter->drawLine(left_, top_, left_ + width_, top_);
                }
            }
            if (fieldObject->borderBottom != QColor(255,255,255,255) && fieldObject->borderBottom != QColor(255,255,255,0)) {
                pen.setColor(fieldObject->borderColor);
                if (painter->isActive()) {
                    painter->setPen(pen);
                    painter->drawLine(left_, top_ + height_, left_ + width_, top_ + height_);
                }
            }
            if (fieldObject->borderLeft != QColor(255,255,255,255) && fieldObject->borderLeft != QColor(255,255,255,0)) {
                pen.setColor(fieldObject->borderColor);
                if (painter->isActive()) {
                    painter->setPen(pen);
                    painter->drawLine(left_, top_, left_, top_ + height_);
                }
            }
            if (fieldObject->borderRight != QColor(255,255,255,255) && fieldObject->borderRight != QColor(255,255,255,0)) {
                pen.setColor(fieldObject->borderColor);
                if (painter->isActive()) {
                    painter->setPen(pen);
                    painter->drawLine(left_ + width_, top_, left_ + width_, top_ + height_);
                }
            }
        }
        if (fieldType == Rhombus) {
            qreal pointX1 = width_/2+left_;
            qreal pointY1 = height_-1+top_;

            qreal pointX2 = width_ + left_;
            qreal pointY2 = height_/2+top_;

            qreal pointX3 = width_/2+left_;
            qreal pointY3 = top_+1;  //

            qreal pointX4 = left_+1;  //
            qreal pointY4 = height_/2+top_;

            QPainterPath path;
            path.moveTo (pointX1, pointY1);
            path.lineTo (pointX2, pointY2);
            path.lineTo (pointX3, pointY3);
            path.lineTo (pointX4, pointY4);
            path.lineTo (pointX1, pointY1);

            QBrush brush(fieldObject->backgroundColor);
            pen.setColor(fieldObject->borderColor);
            if (painter->isActive()) {
                painter->drawPath(path);
                painter->fillPath (path, brush);
            }
        }
        if (fieldType == Triangle) {
            qreal pointX1 = left_;
            qreal pointY1 = height_-1+top_;

            qreal pointX2 = width_+left_;
            qreal pointY2 = height_-1+top_;

            qreal pointX3 = width_/2+left_;
            qreal pointY3 = top_;

            QPainterPath path;
            path.moveTo (pointX1, pointY1);
            path.lineTo (pointX2, pointY2);
            path.lineTo (pointX3, pointY3);
            path.lineTo (pointX1, pointY1);

            QBrush brush(fieldObject->backgroundColor);
            pen.setColor(fieldObject->borderColor);
            if (painter->isActive()) {
                painter->drawPath(path);
                painter->fillPath (path, brush);
            }
        }
        if (fieldType == RoundedReactangle) {
            QRectF rect(left_,top_,width_-2,height_-2);
            QBrush brush(fieldObject->backgroundColor);
            pen.setColor(fieldObject->borderColor);
            if (painter->isActive()) {
                painter->setBrush(brush);
                painter->setPen(pen);
                painter->drawRoundedRect(rect, 40, 40);
            }
        }
        if (fieldType == Reactangle) {
            QRectF rect(left_,top_,width_-2,height_-2);
            QBrush brush(fieldObject->backgroundColor);
            pen.setColor(fieldObject->borderColor);
            if (painter->isActive()) {
                painter->setBrush(brush);
                painter->setPen(pen);
                painter->drawRect(rect);
            }
        }
        if (fieldType == Circle) {
            QBrush brush(fieldObject->backgroundColor);
            pen.setColor(fieldObject->borderColor);
            if (painter->isActive()) {
                painter->setBrush(brush);
                painter->setPen(pen);
                painter->drawEllipse(left_, top_, width_, height_);
            }
        }
        if (fieldType == TextImage || fieldType == DatabaseImage) {  // Proccess field as ImageField
            QImage image = (fieldType == TextImage) ? sectionValueImage(fieldObject->value) : sectionFieldImage(fieldObject->value);

            if (!image.isNull()) {
                QImage scaledImage = image.scaled(QSize(width_,height_), Qt::KeepAspectRatio);
                QPoint point(left_, top_);
                Qt::Alignment alignment = fieldObject->aligment;
                // Horizontal Center
                if (alignment.testFlag(Qt::AlignHCenter)) {
                    int offsetX = (width_ - scaledImage.width()) / 2;
                    point.setX(left_ + offsetX);
                }
                // Vertical Center
                if (alignment.testFlag(Qt::AlignVCenter)) {
                    int offsetY = (height_ - scaledImage.height()) / 2;
                    point.setY(top_ + offsetY);
                }
                if (painter->isActive())
                    painter->drawImage(point,scaledImage);

                if (m_printMode == QtRPT::Html) {
                    QByteArray ba;
                    QBuffer buffer(&ba);
                    buffer.open(QIODevice::WriteOnly);
                    scaledImage.save(&buffer, "PNG"); // writes image into ba in PNG format

                    fieldObject->picture = ba;
                    m_HTML.append(fieldObject->getHTMLStyle());
                }
            }
        }
        if (fieldType == Image) {  // Proccess as static ImageField
            QImage image = QImage::fromData(fieldObject->picture, fieldObject->imgFormat.toLatin1().data());
            if (fieldObject->ignoreAspectRatio == 1) {
                if (painter->isActive())
                    painter->drawImage(QRectF(left_,top_,width_,height_),image);
            } else {
                QImage scaledImage = image.scaled(QSize(width_,height_),
                                                  Qt::KeepAspectRatio/*,
                                                  Qt::SmoothTransformation*/);
                QPoint point(left_, top_);

                if (painter->isActive())
                    painter->drawImage(point,scaledImage);
            }

            if (m_printMode == QtRPT::Html)
                m_HTML.append(fieldObject->getHTMLStyle());
        }
        if (fieldType == Diagram) {
            #if QT_VERSION >= 0x50800
                emit setChart(*fieldObject, *fieldObject->chart);

                GraphDataList dataList = fieldObject->getChartData();
                emit setValueDiagram(dataList);
                fieldObject->setChartData(dataList);

                fieldObject->chart->resize(width_, height_);

                QFont font = fieldObject->chart->legend()->font();
                font.setPointSize(font.pointSize() * 2);
                fieldObject->chart->legend()->setFont(font);

                font = fieldObject->chart->titleFont();
                font.setPointSize(font.pointSize() * 2);
                fieldObject->chart->setTitleFont(font);

                QScopedPointer<QChartView> chartView(new QChartView(fieldObject->chart));
                chartView.data()->setRenderHint(QPainter::TextAntialiasing);
                chartView->show();

                QRectF rect = QRectF(left_, top_, width_, height_);
                chartView.data()->render(painter, rect, chartView.data()->rect());
            #endif
        }
        if (fieldType == Barcode) {
            #ifndef NO_BARCODE
                BarCode br;
                br.setObjectName(fieldObject->objectName());
                QString txt = sectionField(fieldObject->parentBand, fieldObject->value, false, false, "");
                br.setValue(txt);
                BarCode::BarcodeTypes m_barcodeType = (BarCode::BarcodeTypes)fieldObject->barcodeType;
                br.setBarcodeType(m_barcodeType);
                BarCode::FrameTypes m_barcodeFrameType = (BarCode::FrameTypes)fieldObject->barcodeFrameType;
                br.setFrameType(m_barcodeFrameType);
                br.setHeight(fieldObject->barcodeHeight);
                br.drawBarcode(painter,left_,top_,width_,height_);
            #endif
        }
    }
    // Drawing Border, Background, Not Text fields --END--

    if (fieldType == TextRich) {
        QString txt = fieldObject->value;

        QTextDocument document;
        document.setHtml(txt.normalized(QString::NormalizationForm_KC));
        document.setDefaultFont(painter->font());

        int numOfIteration = 0;
        QString alias = findAliasinTHML(&document);
        while (!alias.isEmpty() && numOfIteration < 100) {  //in case of un-expected loop we do no more attempts
            QString value = sectionField(fieldObject->parentBand, alias, true, false, "");
            //qDebug() << alias << value;
            replaceinHTML(&document, alias, value);
            numOfIteration++;
            alias = findAliasinTHML(&document);
        }

        QRectF rect = QRectF(left_+10, top_, width_-15, height_);
        document.setTextWidth( rect.width() );

        if (painter->isActive()) {
            painter->save();
            painter->translate( rect.topLeft() );
        }
        if (draw)
            document.drawContents( painter, rect.translated( -rect.topLeft() ) );
        if (painter->isActive())
            painter->restore();
    }
    if (fieldType == Text) {   // NOT Proccess if field set as ImageField
        setFont(fieldObject);

        QString txt = sectionField(fieldObject->parentBand, fieldObject->value, false, false, fieldObject->formatString, fieldObject->inputFormatString);

        if (draw) {
            QJSValue afterData = m_globalEngine->globalObject().property(fieldObject->objectName() + "AfterData");
            if (!afterData.isNull()) {
                QJSValueList valueList;
                valueList << txt;
                QJSValue result = afterData.call(valueList);

                if (!result.isUndefined())
                    txt = result.toString();
            }
        }

        int flags = fieldObject->aligment | Qt::TextDontClip;
        if (fieldObject->textWrap == 1)
            flags = flags | Qt::TextWordWrap;
        if (draw) {
            pen.setColor(fieldObject->fontColor);

            if (painter->isActive()) {
                painter->setPen(pen);

                if (fieldObject->rotate == 0) {
                    painter->save();
                    painter->translate(left_, top_);
                    QRectF rcT(0, 0, width_, height_);
                    //QRectF rcT(left_, top_, width_, height_);
                    drawText(painter, txt, fieldObject->font, fieldObject->fontColor,
                             rcT, flags, fieldObject->renderingMode, fieldObject->rotate);
                    //painter->drawText(left_, top_, width_, height_, flags, txt);
                    //painter->drawText(left_+10, top_, width_-15, height_, flags, txt);
                    painter->restore();
                }
                if (fieldObject->rotate == 1) {
                    painter->save();
                    painter->translate(left_ + width_, height_);
                    painter->rotate(90);
                    QRectF rcT(0, 0, height_, width_);
                    drawText(painter, txt, fieldObject->font, fieldObject->fontColor,
                             rcT, flags, fieldObject->renderingMode, fieldObject->rotate);
                    //painter->drawText(0, 0, height_, width_, flags, txt);
                    painter->restore();
                }
                if (fieldObject->rotate == 2) {
                    painter->save();
                    painter->translate(left_ + width_, top_ + height_);
                    painter->rotate(180);
                    QRectF rcT(0, 0, height_, width_);
                    drawText(painter, txt, fieldObject->font, fieldObject->fontColor,
                             rcT, flags, fieldObject->renderingMode, fieldObject->rotate);
                    //painter->drawText(0, 0, height_, width_, flags, txt);
                    painter->restore();
                }
                if (fieldObject->rotate == 3) {
                    painter->save();
                    painter->translate(left_, top_ + height_);
                    painter->rotate(-90);
                    //QRectF rcT(0, 0, height_+100, width_+100);
                    QRectF rcT(0, 0, width_, height_);
                    drawText(painter, txt, fieldObject->font, fieldObject->fontColor,
                             rcT, flags, fieldObject->renderingMode, fieldObject->rotate);
                    //painter->drawText(0, 0, height_, width_, flags, txt);
                    painter->restore();
                }
            }

            if (m_printMode == QtRPT::Html)
                m_HTML.append("<div "+fieldObject->getHTMLStyle()+">"+txt+"</div>\n");

            if (m_printMode == QtRPT::Xlsx) {
                RptTabElement element;
                element.fieldObject = fieldObject;
                element.top = top_ * currentPage;
                element.left = left_;
                element.value = txt;
                crossTab->addElement(element);
            }
        } else {
            QRect boundRect = painter->boundingRect(left_+10, top_, width_-15, height_, flags, txt);
            if (boundRect.height() > height_ && fieldObject->autoHeight == 1) {
                /* To correct adjust and display a height of the band we use a param 'realHeight'.
                   Currently this param used only to correct a MasterBand. If will be needed, possible
                   correct also another bands.
                */
                fieldObject->parentBand->realHeight = qRound(boundRect.height()/koefRes_h);
            }
        }
    }
    if (fieldType == CrossTab) {
        if (draw)
        {
            if (fieldObject->crossTab->isMatrixBuilt() == false)
                fieldObject->crossTab->buildMatrix();

            int tmpRowN = -1;  // fact row number
            int prevRow = -1;  // previous row number

            bool isPageHeader = fieldObject->parentBand->type == BandType::PageHeader;
            bool isMasterData = fieldObject->parentBand->type == BandType::MasterData;
            bool isPageFooter = fieldObject->parentBand->type == BandType::PageFooter;

            int startFrom = fieldObject->crossTab->processedCount();
            int fieldsCount = fieldObject->crossTab->fieldList.size();

            for (int nmr = startFrom; nmr < fieldsCount; nmr++) {
                auto field = fieldObject->crossTab->fieldList[nmr];
                int row = fieldObject->crossTab->fieldRow(field, true);

                if (prevRow != row) {
                    tmpRowN += 1;
                    prevRow = row;
                }

                if (tmpRowN > fieldObject->crossTab->visibleRowCount()-1) {
                    // we create a new page only for the particular types of the bands.
                    // And only if No new page will be created from other places

                    if (isPageHeader || isPageFooter) {
                        // Processing other (below) bands before creating a new Page
                        if (isPageHeader) {
                            int y = fieldObject->parentBand->height;
                            if (currentPage == 1)
                                processRTitle(y, draw);
                            processPFooter(draw);
                        }

                        //totalPage++;
                        newPage(printer, bandTop, draw/*, true*/);

                        return;
                    } else if (isMasterData && currentPage >= totalPage) {
                        int y = 0;
                        processPHeader(y, draw);

                        newPage(printer, bandTop, draw);

                        tmpRowN = -1;
                        prevRow = -1;

                        if (prevRow != row) {
                            tmpRowN += 1;
                            prevRow = row;
                        }

                    } else {
                        break;
                    }
                }

                int y = fieldObject->crossTab->rowHeight() * tmpRowN;
                field->rect.setTop(fieldObject->rect.y() + y);
                field->rect.setHeight(fieldObject->crossTab->rowHeight());


                drawFields(field, bandTop, draw);

                fieldObject->crossTab->setProcessedCount(nmr+1);
            }
        }
    }
}

QString QtRPT::findAliasinTHML(QTextDocument *document)
{
    QString plainText = document->toPlainText();

    int startU = plainText.indexOf("<");
    int endU = plainText.indexOf(">");

    int startB = plainText.indexOf("[");
    int endB = plainText.indexOf("]");
    if ((startB < 0 && endB < 0) && (startU < 0 && endU < 0))
        return "";

    if (startU != -1 && startB != -1)
    {
        if (startU < startB)
            return plainText.mid(startU, endU-startU+1);
        else
            return plainText.mid(startB, endB-startB+1);
    }
    else
    {
        if (startU != -1)
            return plainText.mid(startU, endU-startU+1);
        if (startB != -1)
            return plainText.mid(startB, endB-startB+1);
    }


    return QString();

    /*QTextBlock block = document->firstBlock();
    while (block.isValid()) {
        for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
            QTextFragment currentFragment = it.fragment();
            if (!currentFragment.isValid())
                continue;

            if ((currentFragment.text().contains("[") && currentFragment.text().contains("]"))
                || (currentFragment.text().contains("<") && currentFragment.text().contains(">")))
            {
                QString srchTxt = currentFragment.text();
                return srchTxt;
            }
        }

        block = block.next();
    }*/

    return QString();
}

void QtRPT::replaceinHTML(QTextDocument *document, QString alias, QString value)
{
    alias = alias.replace("<","&lt;").replace(">","&gt;");
    QString html = document->toHtml().replace(alias, value);
    document->setHtml(html);
}

void QtRPT::drawLines(RptFieldObject *fieldObject, int bandTop)
{
    if (!isFieldVisible(fieldObject))
        return;

    int startX = fieldObject->lineStartX * koefRes_w;
    int endX = fieldObject->lineEndX * koefRes_w;

    int startY = (bandTop + fieldObject->lineStartY) * koefRes_h;
    int endY = (bandTop + fieldObject->lineEndY) * koefRes_h;

    FieldType fieldType = fieldObject->fieldType;
    QPen pen = getPen(fieldObject);
    pen.setColor(fieldObject->borderColor);
    if (painter->isActive())
        painter->setPen(pen);

    if (fieldType == Line) {
        if (painter->isActive())
            painter->drawLine(startX, startY, endX, endY);
    }

    QLineF line(startX,startY,endX,endY);

    //Draw arrows
    static const double Pi = 3.14159265358979323846264338327950288419717;
    static double TwoPi = 2.0 * Pi;
    double angle = ::acos(line.dx() / line.length());
    if (line.dy() >= 0)
         angle = TwoPi - angle;

     QPointF sourcePoint = line.p1();
     QPointF destPoint = line.p2();
     int arrowSize = 10*koefRes_w;

     if (painter->isActive())
        painter->setBrush(fieldObject->borderColor);

     if (fieldObject->arrowStart == 1) {
         QPointF sourceArrowP1 = sourcePoint + QPointF(sin(angle + Pi / 3) * arrowSize,
                                                       cos(angle + Pi / 3) * arrowSize);
         QPointF sourceArrowP2 = sourcePoint + QPointF(sin(angle + Pi - Pi / 3) * arrowSize,
                                                       cos(angle + Pi - Pi / 3) * arrowSize);
         if (painter->isActive())
            painter->drawPolygon(QPolygonF() << line.p1() << sourceArrowP1 << sourceArrowP2);
     }
     if (fieldObject->arrowEnd == 1) {
         QPointF destArrowP1 = destPoint + QPointF(sin(angle - Pi / 3) * arrowSize,
                                                   cos(angle - Pi / 3) * arrowSize);
         QPointF destArrowP2 = destPoint + QPointF(sin(angle - Pi + Pi / 3) * arrowSize,
                                                   cos(angle - Pi + Pi / 3) * arrowSize);
         if (painter->isActive())
            painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);
     }
}

void QtRPT::drawBandRow(RptBandObject *band, int bandTop, bool allowDraw)
{
    band->realHeight = band->height; //set a 'realHeight' to default value
    /*First pass used to determine a max height of the band*/
    for (const auto &field : band->fieldList)
        if (field->fieldType != Line)
            drawFields(field, bandTop, false);

    /*Second pass used for drawing*/
    if (allowDraw) {
        for (const auto &field : band->fieldList) {
            if (field->fieldType != Line)
                drawFields(field, bandTop, true);
            else
                drawLines(field, bandTop);
        }
    }
}

QVariant QtRPT::processHighligthing(RptFieldObject *field, HiType type)
{
    if (field->highlighting.isEmpty() || field->highlighting.isNull()) {
        switch (type) {
            case FntBold:
                return field->font.bold();
            case FntItalic:
                return field->font.italic();
            case FntUnderline:
                return field->font.underline();
            case FntStrikeout:
                return field->font.strikeOut();
            case FntColor:
                return colorToString(field->m_fontColor);
            case BgColor:
                return colorToString(field->m_backgroundColor);
        }
    } else {
        if (type == BgColor && !field->highlighting.contains("backgroundColor") )
            return colorToString(field->m_backgroundColor);

        QStringList list = field->highlighting.split(";");
        const QString cond = list.at(0);

        for (int i = 1; i < list.size(); i++) {
            if (list.at(i).isEmpty()) continue;
            QString exp = list.at(i);

            RptScriptEngine myEngine(this);

            if (list.at(i).contains("bold") && type == FntBold) {
                exp.remove("bold=");
                QString formulaStr = exp.insert(0,cond);
                formulaStr = getVariableValue(formulaStr, true);
                return myEngine.evaluate(formulaStr).toInt();
            }
            if (list.at(i).contains("italic") && type == FntItalic) {
                exp.remove("italic=");
                QString formulaStr = exp.insert(0,cond);
                formulaStr = getVariableValue(formulaStr, true);
                return myEngine.evaluate(formulaStr).toInt();
            }
            if (list.at(i).contains("underline") && type == FntUnderline) {
                exp.remove("underline=");
                QString formulaStr = exp.insert(0,cond);
                formulaStr = getVariableValue(formulaStr, true);
                return myEngine.evaluate(formulaStr).toInt();
            }
            if (list.at(i).contains("strikeout") && type == FntStrikeout) {
                exp.remove("strikeout=");
                QString formulaStr = exp.insert(0,cond);
                formulaStr = getVariableValue(formulaStr, true);
                return myEngine.evaluate(formulaStr).toInt();
            }
            if (list.at(i).contains("fontColor") && type == FntColor) {
                exp.remove("fontColor=");
                QString formulaStr = exp.insert(1,"'");
                formulaStr = exp.insert(0,cond);
                formulaStr = getVariableValue(formulaStr, true)+"':'"+colorToString(field->m_fontColor)+"'";
                return myEngine.evaluate(formulaStr).toString();
            }
            if (list.at(i).contains("backgroundColor") && type == BgColor) {
                exp.remove("backgroundColor=");
                QString formulaStr = exp.insert(1,"'");
                formulaStr = exp.insert(0,cond);
                formulaStr = getVariableValue(formulaStr, true)+"':'"+colorToString(field->m_backgroundColor)+"'";
                return myEngine.evaluate(formulaStr).toString();
            }
        }
    }
    return QVariant();
}

bool QtRPT::isFieldVisible(RptFieldObject *fieldObject)
{
    bool visible;
    QString formulaStr = fieldObject->printing;
    if (fieldObject->printing.size() > 1) {
        formulaStr = getVariableValue(fieldObject->printing, true);
        RptScriptEngine myEngine(this);
        visible = myEngine.evaluate(formulaStr).toBool();
    } else {
        visible = formulaStr.toInt();
    }
    return visible;
}

QStringList QtRPT::splitStringOnVariable(QString strValue)
{
    QStringList res;

    QString tmpValue = strValue;
    QRegularExpression re("\\[\\D.*?]", QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator i = re.globalMatch(tmpValue);

    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        if (match.hasMatch()) {
             QString variable = match.captured(0);

             res << variable;
        }
    }

    return res;
}

QString QtRPT::getVariableValue(QString scriptStr, bool exp)
{
    // Split string on variables that will be quered
    QStringList varList = splitStringOnVariable(scriptStr);
    for (const auto &variable : varList) {
        QString tmp = sectionValue(variable);  // Query the variable

        if (exp) {   //Process highlighting and visibility
            bool ok;
            tmp.toDouble(&ok);
            if (!ok) tmp.toFloat(&ok);
            if (!ok) tmp.toInt(&ok);
            if (!ok) tmp = "'"+tmp+"'";  //Not a number
        }

        scriptStr = scriptStr.replace(variable, tmp, Qt::CaseInsensitive);
    }

    return scriptStr;
}

QString QtRPT::stringPreprocessing(QString str, QString formatString)
{
    if (str.contains("<Date>"))
        str = str.replace("<Date>", getInternalVariable("Date", formatString).toString());
    if (str.contains("<Time>"))
        str = str.replace("<Time>", getInternalVariable("Time", formatString).toString());
    if (str.contains("<Page>"))
        str = str.replace("<Page>", QString::number(currentPage));
    if (str.contains("<TotalPages>"))
        str = str.replace("<TotalPages>", QString::number(totalPage));
    if (str.contains("<LineNo>"))
        str = str.replace("<LineNo>", getInternalVariable("LineNo", QString()).toString());
    if (str.contains("<LineCount>"))
        str = str.replace("<LineCount>", getInternalVariable("LineCount", QString()).toString());


    str = str.replace("sum([", "SUM([", Qt::CaseInsensitive);
    str = str.replace("avg([", "AVG([", Qt::CaseInsensitive);
    str = str.replace("count([", "COUNT([", Qt::CaseInsensitive);
    str = str.replace("min([", "MIN([", Qt::CaseInsensitive);
    str = str.replace("max([", "MAX([", Qt::CaseInsensitive);

    QRegularExpression re1("(SUM)\\((.*?)\\)", QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator i = re1.globalMatch(str);
    while (i.hasNext()) {
        auto match = i.next();
        if (match.hasMatch()) {
             QString variable = match.captured(0);
             QString tmp = variable;
             tmp = tmp.replace("[", "'").replace("]", "'");
             str = str.replace(variable, tmp, Qt::CaseInsensitive);
        }
    }

    QRegularExpression re2("(AVG)\\((.*?)\\)", QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption);
    i = re2.globalMatch(str);
    while (i.hasNext()) {
        auto match = i.next();
        if (match.hasMatch()) {
             QString variable = match.captured(0);
             QString tmp = variable;
             tmp = tmp.replace("[", "'").replace("]", "'");
             str = str.replace(variable, tmp, Qt::CaseInsensitive);
        }
    }

    QRegularExpression re3("(COUNT)\\((.*?)\\)", QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption);
    i = re3.globalMatch(str);
    while (i.hasNext()) {
        auto match = i.next();
        if (match.hasMatch()) {
             QString variable = match.captured(0);
             QString tmp = variable;
             tmp = tmp.replace("[", "'").replace("]", "'");
             str = str.replace(variable, tmp, Qt::CaseInsensitive);
        }
    }

    QRegularExpression re4("(MIN)\\((.*?)\\)", QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption);
    i = re4.globalMatch(str);
    while (i.hasNext()) {
        auto match = i.next();
        if (match.hasMatch()) {
             QString variable = match.captured(0);
             QString tmp = variable;
             tmp = tmp.replace("[", "'").replace("]", "'");
             str = str.replace(variable, tmp, Qt::CaseInsensitive);
        }
    }

    QRegularExpression re5("(MAX)\\((.*?)\\)", QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption);
    i = re5.globalMatch(str);
    while (i.hasNext()) {
        auto match = i.next();
        if (match.hasMatch()) {
             QString variable = match.captured(0);
             QString tmp = variable;
             tmp = tmp.replace("[", "'").replace("]", "'");
             str = str.replace(variable, tmp, Qt::CaseInsensitive);
        }
    }

    str = str.replace("SUM(","SUM(0,", Qt::CaseInsensitive);
    str = str.replace("AVG(","SUM(1,", Qt::CaseInsensitive);
    str = str.replace("COUNT(","SUM(2,", Qt::CaseInsensitive);
    str = str.replace("MAX(","SUM(3,", Qt::CaseInsensitive);
    str = str.replace("MIN(","SUM(4,", Qt::CaseInsensitive);

    return str;
}

QString QtRPT::sectionField(RptBandObject *band, QString value, bool isReachText, bool firstPass, QString formatString, QString inputFormatString)
{
    QString tmpStr;
    QStringList res;
    bool aggregate = false;

    value = stringPreprocessing(value, formatString);

    // To proccess correctly the different functions/operators of Script language
    // we should make a tmp replacing of '< >' charcters.
    // After spliting on sections, we revert these charactres back.
    // "< if(a < 1) str1; else str2;  if (a > 3) str1; else str2; if   (a = 3) str1; else str2;>";

    // Do searching and tmp replacing - start
    QString tmpValue = value;
    QRegularExpression re("(if\\s*)\\((.*?)\\)", QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = re.match(tmpValue);
    while (match.hasMatch()) {
        //int startOffset = match.capturedStart(1);
        int endOffset = match.capturedEnd(1);

        QString tmpStrFrom = match.capturedTexts().at(0);
        QString tmp        = tmpStrFrom;
        QString tmpStrTo   = tmp.replace("<", "&lt-;").replace(">", "&gt+;");

        //Do tmp replacing
        tmpValue = tmpValue.replace(tmpStrFrom, tmpStrTo);
        match = re.match(tmpValue, endOffset);
    }
    value = tmpValue;
    // Do searching and tmp replacing - end


    QStringList varList = splitStringOnVariable(value);
    for (auto &variable : varList) {
        QString tmp = sectionValue(variable);

        //Process during first pass to calculate aggregate values
        if (firstPass) {
            QString v = variable;
            AggregateValues av;
            av.paramName = v.replace("[", "").replace("]", "");
            av.paramValue = tmp;
            av.lnNo = m_recNo;
            av.pageReport = m_pageReport;
            bool founded = false;

            for (auto &values : listOfPair) {
                if (values.pageReport == av.pageReport &&
                    values.lnNo == av.lnNo &&
                    values.paramName == av.paramName)
                    founded = true;
            }
            if (!founded)
                listOfPair.append(av);
        }

        value.replace(variable, tmp);
    }

    // Split sentence on logical parts
    for (int i = 0; i < value.size(); ++i) {
        if (value.at(i) != '[' && value.at(i) != ']' &&
            value.at(i) != '<' && value.at(i) != '>' && !aggregate)
            tmpStr += value.at(i);
        else if ((value.at(i) == '[' || value.at(i) == ']') && aggregate)
             tmpStr += value.at(i);
        else if (value.at(i) != '<' && value.at(i) != '>' && aggregate)
             tmpStr += value.at(i);
        else {
            if (value.at(i) == ']' && !aggregate) {
                tmpStr += value.at(i);
                res << tmpStr;
                tmpStr.clear();
            }
            if (value.at(i) == '[' && !aggregate) {
                if (!tmpStr.isEmpty())
                    res << tmpStr;
                tmpStr.clear();
                tmpStr += value.at(i);
            }
            if (value.at(i) == '<') {
                aggregate = true;
                if (!tmpStr.isEmpty())
                    res << tmpStr;
                tmpStr.clear();
                tmpStr += value.at(i);
            }
            if (value.at(i) == '>') {
                aggregate = false;
                tmpStr += value.at(i);
                res << tmpStr;
                tmpStr.clear();
            }
        }
    }

    if (!tmpStr.isEmpty())
        res << tmpStr;


    tmpStr.clear();
    for (int i = 0; i < res.size(); ++i) {
        if (!isReachText) {
            // Skip HTML tags
            if (res[i] == "<sub>" || res[i] == "</sub>") isReachText = true;
            if (res[i] == "<sup>" || res[i] == "</sup>") isReachText = true;
            if (res[i] == "<b>" || res[i] == "</b>") isReachText = true;
            if (res[i] == "<i>" || res[i] == "</i>") isReachText = true;
            if (res[i] == "<u>" || res[i] == "</u>") isReachText = true;
            if (res[i] == "<s>" || res[i] == "</s>") isReachText = true;
        }

        if (!isReachText && res[i].contains("<") && res[i].contains(">")) {
            QString formulaStr = res[i];

            RptScriptEngine myEngine(this);
            myEngine.globalObject().setProperty("showInGroup", band->showInGroup);
            myEngine.globalObject().setProperty("groupLevel", band->groupLevel);

            formulaStr = formulaStr.replace("<","");
            formulaStr = formulaStr.replace(">","");

            // Do replacing back
            formulaStr = formulaStr.replace("&lt-;", "<").replace("&gt+;", ">");

            QJSValue result  = myEngine.evaluate(formulaStr);
            res[i] = getFormattedValue(result.toString(), formatString, inputFormatString);
        }

        tmpStr += res.at(i);
    }

    tmpStr = getFormattedValue(tmpStr, formatString, inputFormatString);
    return tmpStr;
}

RptPageObject *QtRPT::getPage(int pageNo)
{
    if (pageNo >= 0 && pageNo < pageList.size())
        return this->pageList.at(pageNo);
    return nullptr;
}

void QtRPT::processGlobalScript()
{
    //qScriptRegisterSequenceMetaType<QList<RptPageObject*> >(m_globalEngine);

    // Put fieldObjects into ScriptEngine
    for (auto &page : pageList) {
        for (auto &band : page->bandList)
            for (auto &field : band->fieldList)
                m_globalEngine->addObject(field);
    }

    QString scriptStr = stringPreprocessing(m_globalScript, "");

    // Get value of variables from user data
    QStringList varList = splitStringOnVariable(scriptStr);
    for (auto &variable : varList) {
        QString tmp = sectionValue(variable);
        scriptStr.replace(variable, tmp);
    }
    qDebug() << "starting evaluate";
    m_globalEngine->evaluate(scriptStr);

    QtRPT *docObject = qobject_cast<QtRPT*>( m_globalEngine->globalObject().property("QtRPT").toQObject() );
    if (docObject == nullptr)
        return;
    else
    {

    }
}

QString QtRPT::getFormattedValue(QString value, QString formatString, QString inputFormatString)
{
    if (!formatString.isEmpty()) {
        // Date format
        if (formatString.at(0) == 'D') {
            QString tmpInpStr = inputFormatString.mid(1,inputFormatString.size()-1);
            if (value.contains('T'))
            {
                auto datetime = QDateTime::fromString(value.trimmed(), "yyyy-MM-ddTHH:mm:ss");
                if (datetime.isValid())
                {
                    value = datetime.toString(formatString.mid(1,formatString.size()-1));
                }
            }
            else
            {
                auto date = QDate::fromString(value.trimmed(), tmpInpStr);
                if (date.isValid())
                {
                    value = date.toString(formatString.mid(1,formatString.size()-1));
                }
            }
        }
        // Numeric format
        if (formatString.at(0) == 'N') {
            bool ok;
            value.toDouble(&ok);
            if (!ok) value.toFloat(&ok);
            if (!ok) value.toInt(&ok);
            if (!ok) return value;

            int precision = formatString.mid(formatString.size()-1,1).toInt();
            QLocale locale;

            if (formatString.mid(1,formatString.size()-2) == "# ###.##") {
                locale = QLocale(QLocale::Ukrainian);
                value = locale.toString(value.toDouble(), 'f', precision).replace(",",".");
            }
            if (formatString.mid(1,formatString.size()-2) == "#,###.##") {
                locale = QLocale(QLocale::Ukrainian);

                value = locale.toString(value.toDouble(), 'f', precision).replace(",",".");
                for (int i = 0; i < value.size(); i++)
                    if (value.at(i).unicode() == 0x00A0)
                        value = value.replace(i, 1 ,",");
            }
            if (formatString.mid(1,formatString.size()-2) == "# ###,##") {
                locale = QLocale("fr_FR");
                value = locale.toString(value.toDouble(), 'f', precision);
            }
            if (formatString.mid(1,formatString.size()-2) == "#.###,##") {
                locale = QLocale(QLocale::German);
                value = locale.toString(value.toDouble(), 'f', precision);
            }
        }
    }

    return value;
}

void QtRPT::fillListOfValue(RptBandObject *bandObject)
{
    for (const auto &field : bandObject->fieldList)
        if (field->fieldType == Text && isFieldVisible(field))
            QString txt = sectionField(bandObject, field->value, false, true);
}

QVariant QtRPT::getInternalVariable(QString value, QString formatString)
{
    if (value.contains("Date")) {
        if (formatString.isEmpty()) {
            QLocale c;
            formatString = c.dateFormat(QLocale::ShortFormat);
        } else {
            if (formatString.at(0) == 'D')
                formatString = formatString.mid(1,formatString.size()-1);
        }
        QString v = QDate::currentDate().toString(formatString);
        return v.replace(0, 1, v[0].toUpper());
    }
    if (value.contains("Time"))
        return QTime::currentTime().toString();
    if (value.contains("Page"))
        return QString::number(currentPage);
    if (value.contains("TotalPages"))
        return QString::number(totalPage);
    if (value.contains("LineNo")) {
        int recNo;
        if (!GroupIdxList_0.isEmpty()) //group processing
            recNo = mg_recNo;
        else //usuall processing
            recNo = m_recNo+1;
        return QString::number(recNo);
    }
    if (value.contains("LineCount")) {
        int maxLnNo = 0;
        for (const auto &pair : listOfPair)
            if (pair.pageReport == m_pageReport && pair.lnNo > maxLnNo)
                maxLnNo = pair.lnNo;

        return maxLnNo+1;
    }

    return QVariant();
}

QImage QtRPT::sectionFieldImage(QString value)
{
    auto rptSql = pageList[m_pageReport]->rtpSql;
    if (rptSql == nullptr)
        return QImage();

    QString fieldName = value;
    fieldName.replace("[","");
    fieldName.replace("]","");
    fieldName.replace(rptSql->objectName()+".","");
    return rptSql->getFieldImage(fieldName, m_recNo);
}

QString QtRPT::sectionValue(QString paramName, int recNo)
{
    paramName.replace("[","");
    paramName.replace("]","");

    if (recNo < 0)
        recNo = m_recNo;

    auto rptSql = pageList[m_pageReport]->rtpSql;
    auto rptDsInline = pageList[m_pageReport]->rptDsInline;

    if (rptSql != nullptr) {
        if (paramName.contains(rptSql->DSName())) {
            QString fieldName = paramName;
            fieldName.replace(rptSql->DSName() + ".", "");
            return rptSql->getFieldValue(fieldName, recNo);
        }
    }
    if (rptDsInline != nullptr) {
        if (paramName.contains(rptDsInline->DSName())) {
            QString fieldName = paramName;
            fieldName.replace(rptDsInline->DSName() + ".", "");
            //qDebug() << "rptDsInline";
            return rptDsInline->getFieldValue(fieldName, recNo);
        }
    }
//    else {
        QVariant paramValue;

        //qDebug() << paramName << recNo << m_pageReport;
        emit setValue(recNo, paramName, paramValue, m_pageReport);
        return paramValue.toString();
//    }

//    return QString();
}

QImage QtRPT::sectionValueImage(QString paramName)
{
    QImage paramValue;
    paramName.replace("[","");
    paramName.replace("]","");
    emit setValueImage(m_recNo, paramName, paramValue, m_pageReport);
    return paramValue;
}

/*void QtRPT::setCallbackFunc( void (*func)(int &recNo, QString &paramName, QVariant &paramValue) ) {
    callbackFunc=func;
}*/

/*!
 \fn QtRPT::printPDF(const QString &filePath, bool open)
 Print report direct to PDF file by \a filePath.
 Second param \a open indicates open or not after printing a pdf file.

 \sa printExec(), printHTML(), printXLSX()
 */
void QtRPT::printPDF(const QString &filePath, bool open)
{
#ifndef QT_NO_PRINTER
    m_printMode = QtRPT::Pdf;
    if (printer == nullptr)
        printer = new QPrinter(m_resolution);

    printer->setOutputFormat(QPrinter::PdfFormat);
    printer->setOutputFileName(filePath);
    if (painter == nullptr)
        painter = new QPainter();

    printPreview(printer);
    if (open)
        QDesktopServices::openUrl(QUrl("file:"+filePath));
#endif
}

/*!
 \fn QtRPT::printHTML(const QString &filePath, bool open)
 Print report direct to HTML file by \a filePath.
 Second param \a open indicates open or not after printing a pdf file.

 \sa printExec(), printPDF(), printXLSX()
 */
void QtRPT::printHTML(const QString &filePath, bool open)
{
#ifndef QT_NO_PRINTER
    m_printMode = QtRPT::Html;
    m_HTML.clear();
    m_HTML.append("<HTML><HEAD><meta charset=\"UTF-8\"></HEAD><BODY>");

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);

    if (printer == nullptr)
        printer = new QPrinter(m_resolution);

    printer->setOutputFormat(QPrinter::PdfFormat);
    if (painter == nullptr)
        painter = new QPainter();

    printPreview(printer);
    m_HTML.append("</BODY></HTML>");

    out << m_HTML;

    file.close();
    if (open)
        QDesktopServices::openUrl(QUrl("file:"+filePath));
#endif
}

/*!
 \fn QtRPT::printODT(const QString &filePath, bool open)
 Print report direct to ODT file by \a filePath.
 Second param \a open indicates open or not after printing a pdf file.

 \warning This function Under construction!

 \sa printExec(), printHTML(), printPDF()
 */
void QtRPT::printXLSX(const QString &filePath, bool open)
{
#ifndef QT_NO_PRINTER
    Q_UNUSED(open);
    Q_UNUSED(filePath);

    #ifdef QXLSX_LIBRARY
        if (crossTab != nullptr)
            delete crossTab;
        crossTab = new RptCrossTabObject();
        crossTab->name = "XLSX_CrosTab";
        m_printMode = QtRPT::Xlsx;

        if (m_xlsx != nullptr)
            delete m_xlsx;
        m_xlsx = new QXlsx::Document(this);

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        if (printer == nullptr)
            printer = new QPrinter(m_resolution);

        printer->setOutputFormat(QPrinter::PdfFormat);
        if (painter == nullptr)
            painter = new QPainter();

        printPreview(printer);

        crossTab->buildXlsx(m_xlsx);

        m_xlsx->saveAs(filePath);

        file.close();
        if (open)
            QDesktopServices::openUrl(QUrl("file:"+filePath));

    #endif
#endif
}

/*!
 \fn QtRPT::printExec(bool maximum, bool direct, QString printerName)
 Open preview of the report in maximize or fitted mode. Or you can print report without preview.
 To open preview in maximize mode, set \a maximum to true.
 \code
 report->printExec(true,false);
 \endcode

 To direct printing without preview dialog, set \a direct to true.
 \code
 report->printExec(true,true);
 \endcode

 If  printer with the \a printerName is not valid,
 the default printer will be used.

 \sa printPDF(), printHTML(), printXLSX()
 */
void QtRPT::printExec(bool maximum, bool direct, QString printerName)
{
#ifndef QT_NO_PRINTER
    m_printMode = QtRPT::Printer;

    if (printer == nullptr)
        printer = new QPrinter(m_resolution);

    if (!printerName.isEmpty() && !printerName.isNull()) {
        printer->setPrinterName(printerName);
        if (!printer->isValid())
            printer->setPrinterName(QPrinterInfo::defaultPrinter().printerName());
    }

    if (painter == nullptr)
        painter = new QPainter();

    if (!direct) {
        QPrintPreviewDialog preview(printer, qobject_cast<QWidget *>(this->parent()), Qt::Window);

        if (maximum) {
            auto list = preview.findChildren<QPrintPreviewWidget *>();
            if (!list.isEmpty()) // paranoiac safety check
                list.first()->setZoomMode(QPrintPreviewWidget::FitToWidth);
        }

        connect(&preview, SIGNAL(paintRequested(QPrinter*)), SLOT(printPreview(QPrinter*)));
        //preview.setWindowState(Qt::WindowMaximized); //Qt BUG https://bugreports.qt-project.org/browse/QTBUG-14517

        auto screen = QGuiApplication::screenAt(preview.mapToGlobal(QPointF(preview.width()/2,0).toPoint()));
        QRect geom = screen->availableGeometry();
        geom.setTop(30);
        geom.setLeft(5);
        geom.setHeight(geom.height()-6);
        geom.setWidth(geom.width()-6);
        preview.setGeometry(geom);

        pr = preview.findChild<QPrintPreviewWidget*>();
        lst = preview.findChildren<QAction*>();

        QIcon icon;
        icon.addPixmap(QPixmap(QString::fromUtf8(":/pdf.png")), QIcon::Normal, QIcon::On);
        auto actExpToPdf = new QAction(icon,tr("Save as PDF"),this);
        actExpToPdf->setObjectName("actExpToPdf");
        connect(actExpToPdf, SIGNAL(triggered()), SLOT(exportTo()));

        icon.addPixmap(QPixmap(QString::fromUtf8(":/html.png")), QIcon::Normal, QIcon::On);
        auto actExpToHtml = new QAction(icon,tr("Save as HTML"),this);
        actExpToHtml->setObjectName("actExpToHtml");
        connect(actExpToHtml, SIGNAL(triggered()), SLOT(exportTo()));

        #ifdef QXLSX_LIBRARY
            QIcon icon1;
            icon1.addPixmap(QPixmap(QString::fromUtf8(":/excel.png")), QIcon::Normal, QIcon::On);
            auto actExpToXlsx = new QAction(icon1,tr("Save as XLSX"),this);
            actExpToXlsx->setObjectName("actExpToXlsx");
            connect(actExpToXlsx, SIGNAL(triggered()), SLOT(exportTo()));
        #endif

        QList<QToolBar*> l1 = preview.findChildren<QToolBar*>();
        l1.at(0)->addAction(actExpToPdf);
        l1.at(0)->addAction(actExpToHtml);
        #ifdef QXLSX_LIBRARY
            l1.at(0)->addAction(actExpToXlsx);
        #endif

        pr->installEventFilter(this);
        //curPage = 1;
        preview.exec();
        preview.setAttribute(Qt::WA_DeleteOnClose);

        connect(&preview, &QPrintPreviewDialog::destroyed, [=] {
            emit previewDestroyed();

            delete printer;
            printer = nullptr;
            delete painter;
            painter = nullptr;
        });
    } else {
        printPreview(printer);  ///print without preview dialog
    }
#endif
}

void QtRPT::exportTo()
{
    QString fileName;
    auto parentWidget = qobject_cast<QWidget *>(this->parent());

    if (sender()->objectName() == "actExpToPdf") {
        fileName = QFileDialog::getSaveFileName(parentWidget, tr("Save File"), "", tr("PDF Files (*.pdf)"));
        if (fileName.isEmpty() || fileName.isNull() )
            return;
        printPDF(fileName,false);
    }
    if (sender()->objectName() == "actExpToHtml") {
        fileName = QFileDialog::getSaveFileName(parentWidget, tr("Save File"), "", tr("HTML Files (*.html)"));
        if (fileName.isEmpty() || fileName.isNull() )
            return;
        printHTML(fileName,true);
    }
    if (sender()->objectName() == "actExpToXlsx") {
        fileName = QFileDialog::getSaveFileName(parentWidget, tr("Save File"), "", tr("XLSX Files (*.xlsx)"));
        if (fileName.isEmpty() || fileName.isNull() )
            return;
        printXLSX(fileName,true);
    }
}

bool lessThan(const AggregateValues a, const AggregateValues b)
{
    if (a.paramValue.toString() == b.paramValue.toString())
        return a.lnNo < b.lnNo;
    else
        return a.paramValue.toString() < b.paramValue.toString();
}

/*!
 \fn QtRPT::printPreview(QPrinter *printer)
 Starts print or draw report at \a printer.
 Commonly, the purpose of this slot is using with QPrintPreviewWidget

 Example:
 \code
    QPrinter *printer = new QPrinter;
    printer->setOutputFormat(QPrinter::PdfFormat);
    printer->setOrientation(QPrinter::Portrait);
    printer->setPaperSize(QPrinter::A4);
    printer->setFullPage(true);

    QPrintPreviewWidget *preview = new QPrintPreviewWidget(printer, this);
    connect(preview, SIGNAL(paintRequested(QPrinter*)), report, SLOT(printPreview(QPrinter*)));
 \endcode
 */
void QtRPT::printPreview(QPrinter *printer)
{
#ifdef QT_NO_PRINTER
    Q_UNUSED(printer);
#else
    if (pageList.isEmpty()) return;
    setPageSettings(printer, 0);

    if (painter == nullptr)
        painter = new QPainter();

    painter->begin(printer);

    fromPage = printer->fromPage();
    toPage   = printer->toPage();


    listOfPair.clear();
    GroupIdxList_0.clear();
    GroupIdxList_1.clear();
    GroupIdxList_2.clear();
    makeReportObjectStructure();

    if (m_userSqlConnection.active)
        setUserSqlConnection(m_userSqlConnection.pageReportNo, m_userSqlConnection);

    m_dataSetInfoList.clear();

    /*Make a two pass report
     *First pass calculate total pages
     *and process some formulas
     *Second pass draw a report
     */

    //First pass
    //processGlobalScript();
    currentPage = 1;
    for (int i = 0; i < pageList.size(); i++) {
        auto reportPage = pageList.at(i);

        quint16 totalOnReportPage = (i == 0) ? currentPage-1 : currentPage;

        for (int ds = 1; ds <= pageList.at(i)->bandsCountByType(MasterData); ds++) {
            DataSetInfo dsInfo;
            dsInfo.dsName = pageList.at(i)->getBand(MasterData, ds)->dsName;
            dsInfo.reportPage = i;
            dsInfo.recordCount = 0;
            dsInfo.dataSetNo = ds;

            emit setDSInfo(dsInfo);

            m_dataSetInfoList << dsInfo;
        }

        m_pageReport = i;
        openDataSource(i);

        GroupIdxList_0.clear();
        GroupIdxList_1.clear();
        GroupIdxList_2.clear();
        m_recNo = 0;

        //First pass
        bool isFirstPage = i == 0 ? true : false;
        processReport(printer, false, i, isFirstPage);
        totalPage = currentPage;

        auto crossTablList = pageList.at(i)->crossTabs();
        for (const auto &field : crossTablList) {
            emit setField(*field);

            if (field->crossTab->isMatrixBuilt() == false)
                field->crossTab->buildMatrix();
        }

        totalOnReportPage = currentPage - totalOnReportPage;

        int crosTabParts = pageList.at(i)->crossTabParts();
        if (crosTabParts > 1)
            totalOnReportPage += crosTabParts - 1;

        reportPage->setTotalPages(totalOnReportPage);
    }

    //Calculating of totalPages, only pages that are visible
    quint16 tlPages = 0;
    for (int i = 0; i < pageList.size(); i++) {
        auto reportPage = pageList.at(i);

        if (reportPage->isVisible())
            tlPages += reportPage->totalPages();
    }

    //Make a correction of the "total pages"
    totalPage = tlPages;

    m_orientation = 0;
    painter->resetTransform();

    //Second pass
    processGlobalScript();
    currentPage = 1;
    bool isFirstPage = true;
    for (int i = 0; i < pageList.size(); i++) {
        auto pageReport = pageList.at(i);
        if (!pageReport->isVisible())
            continue;

        GroupIdxList_0.clear();
        GroupIdxList_1.clear();
        GroupIdxList_2.clear();

        m_recNo = 0;
        m_pageReport = i;

        processReport(printer, true, i, isFirstPage);

        isFirstPage = false;
    }

    painter->end();
#endif
}

void QtRPT::setPageSettings(QPrinter *printer, int pageReportNo)
{
    ph = pageList.at(pageReportNo)->ph;
    pw = pageList.at(pageReportNo)->pw;
    ml = pageList.at(pageReportNo)->ml;
    mr = pageList.at(pageReportNo)->mr;
    mt = pageList.at(pageReportNo)->mt;
    mb = pageList.at(pageReportNo)->mb;

    if (pageList.at(pageReportNo)->watermark) {
        m_backgroundImage = &pageList.at(pageReportNo)->watermarkPixmap;
        setBackgroundImageOpacity(pageList.at(pageReportNo)->watermarkOpacity);
    } else {
        m_backgroundImage = nullptr;
    }

    int orientation = pageList.at(pageReportNo)->orientation;

    QSizeF paperSize;
    paperSize.setWidth(pw/4);
    paperSize.setHeight(ph/4);
    if (printer->printerState() != QPrinter::Active) {
        if (orientation == 1) {
            paperSize.setWidth(ph/4);
            paperSize.setHeight(pw/4);
        }
        QPageSize pageSize(paperSize, QPageSize::Millimeter);
        printer->setPageSize(pageSize);
    }

    QMarginsF margins0(ml/4+0.01, mt/4+0.01, mr/4+0.01, mb/4+0.01);
    printer->setPageMargins(margins0, QPageLayout::Millimeter);
    if (orientation == 1)
    {
        QMarginsF margins1(ml/4+0.01, mr/4+0.01, mt/4+0.01, mb/4+0.01);
        printer->setPageMargins(margins1, QPageLayout::Millimeter);
    }

    #if QT_VERSION >= QT_VERSION_CHECK(5,12,2)
        QRect r = printer->pageLayout().paintRectPixels(printer->resolution());
    #else
        QRect r = printer->pageRect();
    #endif

    //Draw page's border
    if (pageList.at(pageReportNo)->border) {
        if (painter->isActive()) {
            const QPen cpen= painter->pen();
            QPen pen(pageList.at(pageReportNo)->borderColor);
            pen.setWidth(pageList.at(pageReportNo)->borderWidth*5);
            pen.setStyle(getPenStyle(pageList.at(pageReportNo)->borderStyle));
            painter->setPen(pen);
            #if QT_VERSION >= QT_VERSION_CHECK(5,12,2)
                painter->drawRect(0-r.left()+92, 0-r.top()+92,
                                  printer->pageLayout().fullRectPixels(printer->resolution()).width()-192,
                                  printer->pageLayout().fullRectPixels(printer->resolution()).height()-192);   //Rect around page
            #else
                painter->drawRect(0-r.left()+92, 0-r.top()+92,
                                  printer->paperRect().width()-192,
                                  printer->paperRect().height()-192);   //Rect around page
            #endif
            painter->setPen(cpen);
        }
    }

    if (m_orientation != orientation) {
        m_orientation = orientation;
        painter->resetTransform();
        if (orientation == 1) {
            if (painter->isActive()) {
                painter->rotate(90); // поворачиваем относительно (0,0)
                painter->translate(0, -painter->viewport().width()); // смещаемся на ширину вьюпорта влево
            }
        }
    }

    koefRes_h = static_cast<double>(r.height()) / (ph - mt - mb);
    koefRes_w = static_cast<double>(r.width())  / (pw - ml - mr);


    if (orientation == 1) {
        koefRes_h = static_cast<double>(r.width()) / (ph - mt - mb);
        koefRes_w = static_cast<double>(r.height())  / (pw - ml - mr);
    }

    //koefRes_w = koefRes_w + 0.014;
}

void QtRPT::processReport(QPrinter *printer, bool draw, int &pageReportNo, bool isFirstPage)
{
    painter->resetTransform();
    painter->save();

    pageList.at(pageReportNo)->initCrossTabProcessedRows();

    setPageSettings(printer, pageReportNo);
    int y = 0;


    if (!isFirstPage) {
        newPage(printer, y, draw, true);
    } else {
        drawBackground(draw);

        processPHeader(y, draw);

        if (currentPage == 1)
            processRTitle(y, draw);

        processPFooter(draw);//Added 30.11.2019
    }

    processGroupHeader(printer, y, draw, pageReportNo);
    processRSummary(printer, y, draw, pageReportNo);

    painter->restore();
}

/*!
 \fn bool QtRPT::eventFilter(QObject *obj, QEvent *e)
  Filters events if this object has been installed as an event
  filter for the \a obj.

  In your reimplementation of this function, if you want to filter
  the \a e out, i.e. stop it b

  Returns \c value from reimplemented function.

  Reimplemented from QObject::eventFilter()
 */
bool QtRPT::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == pr && e->type()==QEvent::Show) {
        for (auto &action : lst)
            if (action->text().contains("Previous page", Qt::CaseInsensitive))
                action->trigger();

        pr->setCurrentPage(0);
        return true;
    }
    return QObject::eventFilter(obj,e);
}

bool QtRPT::allowPrintPage(bool draw, int curPage_)
{
    if (draw)
    {
        if (curPage_ < fromPage )
            return false;
        if (toPage != 0 && curPage_ > toPage )
            return false;
        return draw;
    }

    return false;
}

bool QtRPT::allowNewPage(bool draw, int curPage_)
{
    if (draw)
    {
        if (currentPage-fromPage < 0)
            return false;
        if (curPage_ < fromPage )
            return false;
        if (toPage != 0 && curPage_ > toPage )
            return false;
        return true;
    }

    return false;
}

void QtRPT::newPage(QPrinter* printer, int &y, bool draw, bool newReportPage)
{
    if (allowNewPage(draw, currentPage+1)) {
        printer->newPage();
        drawBackground(draw);
    }
    currentPage += 1;

    if (draw)
        emit newPage(currentPage);

    if (m_printMode != QtRPT::Html && m_printMode != QtRPT::Xlsx)
        y = 0;

    processPHeader(y,draw);
    if (newReportPage)
        processRTitle(y,draw);

    processPFooter(draw);
}

/*!
 \fn void QtRPT::setBackgroundImageOpacity(float opacity)
 Sets background image opacity from \a opacity
 \sa setBackgroundImage(QPixmap &image)
 */
void QtRPT::setBackgroundImageOpacity(float opacity)
{
    m_backgroundOpacity = opacity;
}

/*!
 \fn QtRPT::setBackgroundImage(QPixmap &image)
 Sets background image from \a image
 \sa setBackgroundImageOpacity(float opacity)
 */
void QtRPT::setBackgroundImage(QPixmap &image)
{
    m_backgroundImage = &image;
}

/*! \overload
 Sets background image from \a image
 \sa setBackgroundImageOpacity(float opacity)
*/
void QtRPT::setBackgroundImage(QPixmap image)
{
    m_backgroundImage = &image;
}

void QtRPT::drawBackground(bool draw)
{
    if (!draw)
        return;

    if (painter->isActive())
        painter->setBackgroundMode(Qt::TransparentMode);

    if (m_backgroundImage != nullptr) {
        if (painter->isActive()) {
            painter->setOpacity (m_backgroundOpacity);

            painter->drawPixmap(0, //-ml*koefRes_w,
                                0, //-mt*koefRes_h,
                                pw*koefRes_w - ml*koefRes_w*2,
                                ph*koefRes_h - mt*koefRes_h*2,
                                *m_backgroundImage);
            painter->setOpacity (1.0);
        }
    }
}

void QtRPT::processGroupHeader(QPrinter *printer, int &y, bool draw, int &pageReportNo)
{
    if (currentPage == 0)
        processPFooter(draw);
    qDebug() << "***********************************************************************";
    for (int dsNo = 1; dsNo < 6; dsNo++) {
        m_recNo = 0;
        mg_recNo = 0;

        auto bandDGH_0 = pageList.at(pageReportNo)->getBand(DataGroupHeader, dsNo);
        auto bandDGH_1 = pageList.at(pageReportNo)->getBand(DataGroupHeader, dsNo, 1);
        auto bandDGH_2 = pageList.at(pageReportNo)->getBand(DataGroupHeader, dsNo, 2);
        auto bandDGF_0 = pageList.at(pageReportNo)->getBand(DataGroupFooter, dsNo, 0);
        auto bandDGF_1 = pageList.at(pageReportNo)->getBand(DataGroupFooter, dsNo, 1);
        auto bandDGF_2 = pageList.at(pageReportNo)->getBand(DataGroupFooter, dsNo, 2);
        auto bandMH    = pageList.at(pageReportNo)->getBand(MasterHeader, dsNo);
        auto bandMD    = pageList.at(pageReportNo)->getBand(MasterData, dsNo);
        auto bandMF    = pageList.at(pageReportNo)->getBand(MasterFooter, dsNo);
        auto bandPF    = pageList.at(pageReportNo)->getBand(PageFooter, dsNo);

        QStringList groupList;
        // Заполняем данные в массив, который будем использовать для разбиения на группы
        if (bandMD) {
            if (!bandMD->groupingField.isEmpty()) {
                groupList = bandMD->groupingField.split(",");

                fillListOfValue(bandMD);

                int recCount = 0;
                if (bandMD->dsName.isEmpty())
                    recCount = getRecCount(pageReportNo, dsNo);
                else
                    recCount = getRecCount(pageReportNo, bandMD->dsName);

                if (recCount >= pageReportNo+1) {
                    for (int i = 0; i < recCount; i++) {
                        m_recNo = i;
                        for (const auto &groupField : groupList)
                            sectionField(bandMD, groupField, false, true);
                    }
                }
            }
        }

        if (groupList.size() == 0) {
            processMHeader(y, dsNo, draw, pageReportNo);
            processMasterData(printer, y, draw, pageReportNo, dsNo, QList<int>());
            processMFooter(printer, y, dsNo, draw, pageReportNo);
        } else {
            if (!listOfPair.isEmpty()) {
                //new
                rowList.clear();

                // Формируем массив групп
                for (int lvl = 0; lvl < groupList.size(); lvl++) {
                    QString groupingFieldName = groupList.at(lvl);
                    groupingFieldName.replace("[", "").replace("]", "");
                    //qDebug() << "---" << groupingFieldName << "---";

                    for (int j = 0; j < listOfPair.size(); ++j) {
                        if (listOfPair.at(j).pageReport == pageReportNo &&
                            listOfPair.at(j).paramName == groupingFieldName)
                        {
                            bool found = false;
                            for (auto &rowData : rowList) {
                                if (rowData.ln_no == listOfPair.at(j).lnNo) {
                                    if (lvl == 0) {
                                        rowData.lvl0_data = listOfPair.at(j).paramValue.toString();
                                        found = true;
                                    }
                                    if (lvl == 1) {
                                        rowData.lvl1_data = listOfPair.at(j).paramValue.toString();
                                        found = true;
                                    }
                                    if (lvl == 2) {
                                        rowData.lvl2_data = listOfPair.at(j).paramValue.toString();
                                        found = true;
                                    }
                                }
                            }

                            if (!found) {
                                RowData rowData;
                                rowData.ln_no = listOfPair.at(j).lnNo;
                                if (lvl == 0)
                                    rowData.lvl0_data = listOfPair.at(j).paramValue.toString();
                                if (lvl == 1)
                                    rowData.lvl1_data = listOfPair.at(j).paramValue.toString();
                                if (lvl == 2)
                                    rowData.lvl2_data = listOfPair.at(j).paramValue.toString();
                                rowList << rowData;
                            }
                        }
                    }
                }

                // Сортируем
                if (bandMD->sortDataInGroup)
                {
                    std::sort(rowList.begin(), rowList.end(),[](RowData const &l, RowData const &r)
                    {
                        if (l.lvl0_data != r.lvl0_data)
                            return l.lvl0_data < r.lvl0_data;

                        if (l.lvl1_data != r.lvl1_data)
                            return l.lvl1_data < r.lvl1_data;

                        return l.lvl2_data < r.lvl2_data;
                    });
                }

                if (bandMH != nullptr && bandMH->showInGroup == 0)
                    processMHeader(y, dsNo, draw, pageReportNo);

                /*for (int rd = 0; rd < rowList.size(); rd++) {
                    qDebug() << rowList.at(rd).ln_no << rowList.at(rd).lvl0_data<< rowList.at(rd).lvl1_data << rowList.at(rd).lvl2_data;
                }*/


                // Split on groups
                int grpNo = 0;
                bool isGrStart_0 = true;
                bool isGrStart_1 = false;
                bool isGrStart_2 = false;
                QList<int> GroupIdxList_current;
                for (int rd = 0; rd < rowList.size(); rd++) {
                    auto rowData = rowList.at(rd);

                    bool isGrEnd_0 = false;
                    bool isGrEnd_1 = false;
                    bool isGrEnd_2 = false;

                    if (rd == rowList.size() - 1)
                    {
                        isGrEnd_0 = true;
                        isGrEnd_1 = true;
                        isGrEnd_2 = true;
                    }
                    else if (rd > -1)
                    {
                        if (rowData.lvl0_data != rowList.at(rd+1).lvl0_data)
                            isGrEnd_0 = true;
                        if (rowData.lvl1_data != rowList.at(rd+1).lvl1_data)
                            isGrEnd_1 = true;
                        if (rowData.lvl2_data != rowList.at(rd+1).lvl2_data)
                            isGrEnd_2 = true;
                    }

                    //qDebug() << isGrEnd_0  << isGrEnd_1 << isGrEnd_2
                    //         << rowData.ln_no << rowData.lvl0_data << rowData.lvl1_data ;

                    GroupIdxList_0 << rowData.ln_no;
                    GroupIdxList_1 << rowData.ln_no;
                    GroupIdxList_2 << rowData.ln_no;
                    GroupIdxList_current << rowData.ln_no;

                    //qDebug() <<  "CurrentGroup" << GroupIdxList_current;

                    if (!isGrEnd_0 && !isGrEnd_1 && !isGrEnd_2) {
                        //qDebug() << "continue";
                        continue;
                    }

                    //qDebug() << GroupIdxList_0 << GroupIdxList_1 << "ALL: " << GroupIdxList_current;

                    //-----------------
                    // Start new group
                    {
                        grpNo += 1;

                        int yPF = 0;

                        if (bandPF != nullptr)
                            yPF = bandPF->height;

                        int yMF = 0;
                        if (bandMF != nullptr)
                            yMF = bandMF->height;

                        m_recNo = rowData.ln_no;

                        // Start new page for each Data group, only for level 0
                        if (isGrStart_0)
                            if (bandMD->startNewPage == 1 && grpNo != 1)
                                newPage(printer, y, draw);

                        // DGH only for level 0
                        if (bandDGH_0 != nullptr && (isGrStart_0 || bandDGH_0->groupHeaderEachlevel || isGrStart_1 || isGrStart_2 )) {
                            if (y + bandDGH_0->height > ph-mb-mt-yPF-yMF)
                                newPage(printer, y, draw);

                            if (allowPrintPage(draw,currentPage))  //Draw header of the group
                                drawBandRow(bandDGH_0, y);
                            y += bandDGH_0->height;
                        }

                        if (bandMH != nullptr && bandMH->showInGroup == 1)
                            processMHeader(y, dsNo, draw, pageReportNo);


                        processMasterData(printer, y, draw, pageReportNo, dsNo, GroupIdxList_current);

                        if (bandMF != nullptr && bandMF->showInGroup == 1)
                            processMFooter(printer, y, dsNo, draw, pageReportNo);

                        m_recNo = rowData.ln_no;

                        // Формируем футеры груп
                        if (isGrEnd_0)
                        {
                            isGrEnd_1 = true;
                            isGrEnd_2 = true;
                        }
                        else if (isGrEnd_1)
                        {
                            isGrEnd_2 = true;
                        }

                        // Второй уровень
                        if (isGrEnd_2 && bandDGF_2 != nullptr) {
                            if (y + bandDGF_2->height > ph-mb-mt-yPF-yMF)
                                newPage(printer, y, draw);
                        }

                        if (isGrEnd_2 && bandDGF_2 != nullptr) {
                            if (allowPrintPage(draw,currentPage))  //Draw footer of the group
                                drawBandRow(bandDGF_2, y);

                            y += bandDGF_2->height;
                        }

                        // Первый уровень
                        if (isGrEnd_1 && bandDGF_1 != nullptr) {
                            if (y + bandDGF_1->height > ph-mb-mt-yPF-yMF)
                                newPage(printer, y, draw);
                        }

                        if (isGrEnd_1 && bandDGF_1 != nullptr) {
                            if (allowPrintPage(draw,currentPage))  //Draw footer of the group
                                drawBandRow(bandDGF_1, y);

                            y += bandDGF_1->height;
                        }

                        // Нулевой уровень
                        if (isGrEnd_0 && bandDGF_0 != nullptr) {
                            if (y + bandDGF_0->height > ph-mb-mt-yPF-yMF)
                                newPage(printer, y, draw);
                        }

                        if (isGrEnd_0 && bandDGF_0 != nullptr) {
                            if (allowPrintPage(draw,currentPage))  //Draw footer of the group
                                drawBandRow(bandDGF_0, y);

                            y += bandDGF_0->height;
                        }

                    }
                    //-----------------


                    isGrStart_0 = false;

                    if (isGrEnd_2) {
                        GroupIdxList_2.clear();
                    }
                    if (isGrEnd_1) {
                        GroupIdxList_1.clear();
                    }
                    if (isGrEnd_0) {
                        GroupIdxList_0.clear();

                        isGrStart_0 = true;

                        // Reset numeration for group
                        if (bandMD->startNewNumeration != 0)
                            mg_recNo = 0;
                    }

                    GroupIdxList_current.clear();
                }

                if (bandMF != nullptr && bandMF->showInGroup == 0)
                    processMFooter(printer, y, dsNo, draw, pageReportNo);
                //------------------
            }
        }
    }
}

void QtRPT::processMasterData(QPrinter *printer, int &y, bool draw, int &pageReportNo,
                              int dsNo, QList<int> GroupIdxList_current)
{
    auto bandMD  = pageList.at(pageReportNo)->getBand(MasterData, dsNo);
    auto bandMF  = pageList.at(pageReportNo)->getBand(MasterFooter, dsNo);
    auto bandPF  = pageList.at(pageReportNo)->getBand(PageFooter, dsNo);

    int recCount = 0;

    if (bandMD) {
        if (bandMD->dsName.isEmpty())
            recCount = getRecCount(pageReportNo, dsNo);
        else
            recCount = getRecCount(pageReportNo, bandMD->dsName);
    }


    if (recCount > 0) {
        if (pageList.at(pageReportNo)->getBand(MasterData, dsNo) != nullptr) {
            for (int i = 0; i < recCount; i++) {
                m_recNo = i;

                bool found = false;
                //If report with groups, we checking that current line in the current group
                if (!GroupIdxList_current.isEmpty()) {
                    if (GroupIdxList_current.indexOf(i) != -1)
                        found = true;
                }  else {
                    found = true;
                }

                if (found) {
                    mg_recNo += 1;
                    int yPF = 0;
                    if (bandPF != nullptr)
                        yPF = bandPF->height;

                    int yMF = 0;
                    if (bandMF != nullptr)
                        yMF = bandMF->height;

                    drawBandRow(bandMD, y, false);                    //1101

                    if (y + bandMD->realHeight > ph-mb-mt-yPF-yMF) {  //901+20 > 1188-40-40-200-0
                        if (m_printMode != QtRPT::Html) {
                            processMFooter(printer, y, dsNo, draw, pageReportNo);
                            newPage(printer, y, draw);
                            processMHeader(y, dsNo, draw, pageReportNo);
                        }
                    }

                    if (allowPrintPage(draw,currentPage))
                        drawBandRow(bandMD, y, true);
                    else
                        fillListOfValue(bandMD);

                    y += bandMD->realHeight;
                }
            }
        }
    }
}

void QtRPT::processMHeader(int &y, int dsNo, bool draw, int &pageReportNo)
{
    auto bandMH  = pageList.at(pageReportNo)->getBand(MasterHeader, dsNo);

    if (bandMH == nullptr) return;

    if (allowPrintPage(draw, currentPage))
        drawBandRow(bandMH, y);

    y += bandMH->height;

    //painter.drawLine(0,y*koefRes_h,r.width(),y*koefRes_h);
}

void QtRPT::processRTitle(int &y, bool draw)
{
    auto bandRT = pageList.at(m_pageReport)->getBand(ReportTitle, 1);

    if (bandRT == nullptr) return;

    if (allowPrintPage(draw, currentPage))
        drawBandRow(bandRT, y);

    y += bandRT->height;

    //painter.drawLine(0,y*koefRes_h,r.width(),y*koefRes_h);
}

void QtRPT::processPHeader(int &y, bool draw)
{
    auto bandPH = pageList.at(m_pageReport)->getBand(PageHeader, 1);

    if (bandPH == nullptr) return;

    if (m_printMode == QtRPT::Html) return;

    if (allowPrintPage(draw, currentPage))
        drawBandRow(bandPH, y);

    y += bandPH->height;

    //painter.drawLine(0,y*koefRes_h,pw*koefRes_h,y*koefRes_h);
}

void QtRPT::processMFooter(QPrinter *printer, int &y, int dsNo, bool draw, int &pageReportNo)
{
    auto bandMF  = pageList.at(pageReportNo)->getBand(MasterFooter, dsNo);

    if (bandMF == nullptr) return;

    if (y > ph-mb-mt-bandMF->height)
        newPage(printer, y, draw);

    if (allowPrintPage(draw,currentPage))
        drawBandRow(bandMF, y);

    y += bandMF->height;
}

void QtRPT::processPFooter(bool draw)
{
    auto bandPF = pageList.at(m_pageReport)->getBand(PageFooter, 1);

    if (bandPF == nullptr) return;

    if (m_printMode == QtRPT::Html) return;

    int y1 = ph-mb-mt-bandPF->height;

    if (allowPrintPage(draw, currentPage))
        drawBandRow(bandPF, y1);

    //painter.drawLine(0,y1*koefRes_h,pw*koefRes_h,y1*koefRes_h);
}

void QtRPT::processRSummary(QPrinter *printer, int &y, bool draw, int &pageReportNo)
{
    auto bandRS = pageList.at(pageReportNo)->getBand(ReportSummary, 1);

    if (bandRS == nullptr) return;

    if (y + bandRS->height > ph-mb-mt)
        newPage(printer, y, draw);

    if (allowPrintPage(draw, currentPage))
        drawBandRow(bandRS, y);

    y += bandRS->height;

    //painter.drawLine(0,y*koefRes_h,pw*koefRes_h,y*koefRes_h);
}

void QtRPT::openDataSource(int &pageReportNo)
{
    RptSqlConnection sqlConnection = pageList[pageReportNo]->sqlConnection;

    if (sqlConnection.active) {
        // If user connection is active, use their parameters
        auto rptSql = new RptSql(this);
        rptSql->setConnection(sqlConnection);
        rptSql->setObjectName(sqlConnection.dsName);

        pageList[pageReportNo]->rtpSql = rptSql;

        if (rptSql->openQuery()) {
            // Подразумеваем что у нас dsSetNo = 1
            // в будущем переделать по имени
            if (!setRecCount(pageReportNo, rptSql->DSName(), rptSql->getRecordCount()))
                if (!setRecCount(pageReportNo, 1, rptSql->getRecordCount()))
                {
                    DataSetInfo dsInfo;
                    dsInfo.dsName = rptSql->DSName();
                    dsInfo.reportPage = pageReportNo;
                    dsInfo.recordCount = rptSql->getRecordCount();
                    dsInfo.dataSetNo = 1;

                    m_dataSetInfoList << dsInfo;
                }
        }
    } else {
        QDomElement repElem = m_xmlDoc.elementsByTagName("Report").at(pageReportNo).toElement();
        QDomNode n = repElem.firstChild();

        // Load all DS for report page
        QVector<QDomElement> dsList;
        while(!n.isNull()) {
            QDomElement e = n.toElement();
            //qDebug() << e.tagName();
            if (!e.isNull() && e.tagName() == "DataSource")
                dsList << e;

            n = n.nextSibling();
        }

        for (const auto &dsElement : dsList) {
            if (!dsElement.isNull() && dsElement.attribute("type") == "SQL") {
                auto rptSql = new RptSql(this);
                rptSql->loadXML(dsElement);
                pageList[pageReportNo]->rtpSql = rptSql;

                if (rptSql->openQuery()) {
                    // Подразумеваем что у нас dsSetNo = 1
                    // в будущем переделать по имени
                    if (!setRecCount(pageReportNo, rptSql->DSName(), rptSql->getRecordCount()))
                        if (!setRecCount(pageReportNo, 1, rptSql->getRecordCount()))
                        {
                            DataSetInfo dsInfo;
                            dsInfo.dsName = rptSql->DSName();
                            dsInfo.reportPage = pageReportNo;
                            dsInfo.recordCount = rptSql->getRecordCount();
                            dsInfo.dataSetNo = 1;

                            m_dataSetInfoList << dsInfo;
                        }
                }
            }

            if (!dsElement.isNull() && dsElement.attribute("type") == "XML") {

            }

            if (!dsElement.isNull() && dsElement.attribute("type") == "INLINE") {

                RptDsAbstract *abstactDS = nullptr;

                if (dsElement.attribute("plugin", "").isEmpty()) {
                    auto rptDsInline = new RptDsInline(this);
                    abstactDS = rptDsInline;
                    rptDsInline->loadXML(dsElement);
                }
                if (!dsElement.attribute("plugin", "").isEmpty()) {
                    auto rptDsPlugin = new RptDsPlugin(this);
                    abstactDS = rptDsPlugin;
                    rptDsPlugin->loadXML(dsElement);
                }

                if (abstactDS) {
                    if (!setRecCount(pageReportNo, abstactDS->DSName(), abstactDS->getRecordCount()))
                    {
                        DataSetInfo dsInfo;
                        dsInfo.dsName = abstactDS->DSName();
                        dsInfo.reportPage = pageReportNo;
                        dsInfo.recordCount = abstactDS->getRecordCount();
                        dsInfo.dataSetNo = 1;

                        m_dataSetInfoList << dsInfo;
                    }

                    pageList[pageReportNo]->rptDsInline = abstactDS;
                }
            }
        }
    }
}

/*!
 \fn void QtRPT::newPage(int page)
  This signal is emitted when the new page generated.
  \a page is a number of new page.
 */

/*!
 \fn void QtRPT::setField(RptFieldObject &fieldObject)
  This signal is emitted when QtRPT request a Field's data from user application.
  Pass \a fieldObject to user's application as a reference to requested RptFieldObject.
  User should set the appropriate properties of the \a fieldObject.
  This signal is emitted before following signals: setValue, setValueImage, setValueDiagram
  \sa setValue(), setValueImage(), setValueDiagram(), RptFieldObject
 */

/*!
 \fn void QtRPT::setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage)
  This signal is emitted when QtRPT request a Field's data from user application.
  And pass to the user's application the following variables
    \list
    \li \a recNo - record number (if field placed on DataBand)
    \li \a paramName - name of requested field
    \li \a reportPage - number of the report
    \endlist
  From user's applications, QtRPT expects the data in the variable \a paramValue, which acts as a references
    \list
    \li \a paramValue - data, which will processed by QtRPT and printed
    \endlist
  This signal is emitted after following signal: setField()
  \sa setField(), setValueImage(), setValueDiagram()
 */

/*!
 \fn void QtRPT::setValueImage(const int recNo, const QString paramName, QImage &paramValue, const int reportPage);
  This signal is emitted when QtRPT request a Field's data from user application.
  And pass to the user's application the following variables
    \list
    \li \a recNo - record number (if field placed on DataBand)
    \li \a paramName - name of requested field
    \li \a reportPage - number of the report
    \endlist
  From user's applications, QtRPT expects the QImage in the variable \a paramValue, which acts as a references
    \list
    \li \a paramValue - QImage, which will processed by QtRPT and printed
    \endlist
  This signal is emitted after following signal: setField()
  \sa setField(), setValue(), setValueDiagram()
 */

/*!
 \fn void QtRPT::setValueDiagram(Chart &chart);
  This signal is emitted when QtRPT request a Chart's data from user application.
  Pass \a chart to user's application as a reference to requested Chart object.
  User should set the appropriate properties of the \a chart.
  This signal is emitted after following signal: setField()
  \sa setField(), setValue(), setValueImage()
 */

/*!
 \fn void QtRPT::setChart(RptFieldObject &fieldObject, QChart *chart);
  This signal is emitted when QtRPT request a QChart from user application.
  Pass \a fieldObject to user's application as a reference that hold the chart object.
  Pass \a chart to user's application as a pointer to requested QChart object.
  User should set the appropriate properties of the \a chart.
  Please note, you must have installed Qt 5.8.0 and higher
  This signal is emitted after following signal: setField()
  \sa setField(), setValue(), setValueImage()
 */

/*!
  \page qtrptproject.html
  \title QtRptProject
  \list
  \li Version 2.2.0
  \li Programmer: Aleksey Osipov
  \li Web-site: \l {http://www.aliks-os.tk} {http://www.aliks-os.tk}
  \li Email: \l {mailto:aliks-os@ukr.net} {aliks-os@ukr.net}
  \li Web-site: \l {https://qtrpt.sourceforge.io} {https://qtrpt.sourceforge.io}
  \li Address in Facebook \l {https://www.facebook.com/qtrpt} {https://www.facebook.com/qtrpt}
  \endlist
  QtRPT is the easy-to-use print report engine written in C++ QtToolkit. It allows combining several reports in one XML file. For separately taken field, you can specify some condition depending on which this field will display in different font and background color, etc. The project consists of two parts: report library QtRPT and report designer application QtRptDesigner. Report file is a file in XML format. The report designer makes easy to create report XML file. Thanks to Qt library, our project can be used in programs for work in the operating systems Windows, Linux, MacOS

  \contentspage {Basic Qt} {Contents}
  Table of contents:
  \list
  \li \l {QtRptName} {QtRptName Namespace}
  \li \l {QtRPT} {QtRPT class}
  \li \l {RptPageObject} {RptPageObject class}
  \li \l {RptBandObject} {RptBandObject class}
  \li \l {RptFieldObject} {RptFieldObject class}
  \endlist
*/

void QtRPT::getUserSqlConnection(int &pageReportNo, RptSqlConnection &sqlConnection)
{
    if (pageList.size() <= pageReportNo)  // Return inactive connection
        sqlConnection = pageList[pageReportNo]->sqlConnection;
}

void QtRPT::setUserSqlConnection(int &pageReportNo, const RptSqlConnection &sqlConnection)
{
    pageList[pageReportNo]->sqlConnection = sqlConnection;
}

void QtRPT::setUserSqlConnection(int pageReportNo, QString dsName, QString dbType, QString dbName,
                                 QString dbHost, QString dbUser, QString dbPassword, int dbPort,
                                 QString dbConnectionName, QString sqlQuery, QString dbCoding,
                                 QString charsetCoding)
{
    m_userSqlConnection.dbHost = dbHost;
    m_userSqlConnection.dbPort = dbPort;
    m_userSqlConnection.dbUser = dbUser;
    m_userSqlConnection.dbPassword = dbPassword;
    m_userSqlConnection.dbName = dbName;
    m_userSqlConnection.dbType = dbType;
    m_userSqlConnection.dbConnectionName = dbConnectionName;
    m_userSqlConnection.sqlQuery = sqlQuery;
    m_userSqlConnection.dsName = dsName;
    m_userSqlConnection.dbCoding = dbCoding;
    m_userSqlConnection.charsetCoding = charsetCoding;
    m_userSqlConnection.active = true;
    m_userSqlConnection.pageReportNo = pageReportNo;


}

void QtRPT::activateUserSqlConnection(int pageReportNo, bool bActive)
{
    // Enable or disable connection
    pageList[pageReportNo]->sqlConnection.active = bActive;
}
