#include "Barcode.h"
#include <QDebug>
#include <QLibrary>
#ifndef NO_BARCODE
    #include "qzint.h"
#endif

BarCode::BarCode(QWidget *parent)
: QWidget(parent)
{
    #ifndef NO_BARCODE
        bc = 0;
        QLibrary library("QtZint");
        if (!library.load())
            qDebug() << library.errorString();

        typedef Zint::QZint*(*CreateZint)();
        CreateZint cwf = (CreateZint)library.resolve("createWidget");
        if (cwf)
            bc = cwf();
        else
            qDebug() << "Could not create Zint from the loaded library";

    #endif
    m_value = "QtRPT";
    m_BarcodeType = CODE128; //CODE128
    m_FrameType = NO_BORDER; //NO FRAME
    m_height = 50;
}

void BarCode::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    #ifndef NO_BARCODE
        if (bc == nullptr)
            painter.drawText(QPointF(0,0),"Zint library not found");
        else
            drawBarcode(&painter, 0, 0, this->width(), this->height());

    #else
        painter.drawText(QPointF(0,0),"NO_BARCODE");
    #endif
}

void BarCode::drawBarcode(QPainter *painter, qreal x, qreal y, qreal width, qreal height)
{
    #ifndef NO_BARCODE
        if (bc == nullptr) {
            painter->drawText(QRectF(x,y,width,height), Qt::AlignCenter,"Zint library not found");
            return;
        }
        bc->setSymbol(m_BarcodeType);

        //bc.setPrimaryMessage("dprimaryMessage");  //???
        bc->setBorderType((Zint::QZint::BorderType)m_FrameType);  //Тип обрамляющей рамки
        //bc->setHeight(height);
        bc->setWidth(width);
        bc->setSecurityLevel(0);
        //bc.setBorderWidth(1);
        /*bc.setPdf417CodeWords(d->pdf417Max);
        bc.setFgColor(d->barcodeColor);*/
        bc->setBgColor(QColor(0,0,0,0));
        bc->setText(m_value);
        bc->setWhitespace(5);

        bc->render(*painter,QRectF(x,y,width,height), Zint::QZint::KeepAspectRatio);
    #else
        painter->drawText(QRectF(x,y,width,height), Qt::AlignCenter,"NO_BARCODE");
    #endif
}

void BarCode::setBarcodeType(BarcodeTypes value)
{
    m_BarcodeType = value;
    #ifndef NO_BARCODE
        if (bc != nullptr)
            bc->setSymbol(value);
        repaint();
    #endif
}

void BarCode::setFrameType(FrameTypes value)
{
    m_FrameType = value;
    #ifndef NO_BARCODE
        if (bc != nullptr)
            bc->setBorderType((Zint::QZint::BorderType)m_FrameType);
        repaint();
    #endif
}

void BarCode::setHeight(int value)
{
    m_height = value;
    #ifndef NO_BARCODE
        if (bc != 0)
            bc->setHeight(value);
        repaint();
    #endif
}

BarCode::BarcodeTypePairList BarCode::getTypeList()
{
    BarcodeTypePairList list;
    const QMetaObject &mo = staticMetaObject;
    for (int i=0; i < mo.enumerator(0).keyCount(); i++) {
        QPair<BarcodeTypes, QString> pair;
        pair.first = (BarcodeTypes)mo.enumerator(0).value(i);
        pair.second = getTypeNameList()[i];
        list.append(pair);
    }
    return list;
}

QString BarCode::getTypeName(BarcodeTypes type)
{
    for (const auto &pair : getTypeList())
        if (pair.first == type)
            return pair.second;

    return QString();
}

const QStringList BarCode::getTypeNameList()
{
    QStringList bstyle_text;
        bstyle_text<<"CODE11";
        bstyle_text<<"C25MATRIX";
        bstyle_text<<"C25INTER";
        bstyle_text<<"C25IATA";
        bstyle_text<<"C25LOGIC";
        bstyle_text<<"C25IND";
        bstyle_text<<"CODE39";
        bstyle_text<<"EXCODE39";
        bstyle_text<<"EANX";
        bstyle_text<<"EAN128";
        bstyle_text<<"CODABAR";
        bstyle_text<<"CODE128";
        bstyle_text<<"DPLEIT";
        bstyle_text<<"DPIDENT";
        bstyle_text<<"CODE16K";
        bstyle_text<<"CODE93";
        bstyle_text<<"FLAT";
        bstyle_text<<"RSS14";
        bstyle_text<<"RSS_LTD";
        bstyle_text<<"RSS_EXP";
        bstyle_text<<"TELEPEN";
        bstyle_text<<"UPCA";
        bstyle_text<<"UPCE";
        bstyle_text<<"POSTNET";
        bstyle_text<<"MSI_PLESSEY";
        bstyle_text<<"FIM";
        bstyle_text<<"LOGMARS";
        bstyle_text<<"PHARMA";
        bstyle_text<<"PZN";
        bstyle_text<<"PHARMA_TWO";
        bstyle_text<<"PDF417";
        bstyle_text<<"PDF417TRUNC";
        bstyle_text<<"MAXICODE";
        bstyle_text<<"QRCODE";
        bstyle_text<<"CODE128B";
        bstyle_text<<"AUSPOST";
        bstyle_text<<"AUSREPLY";
        bstyle_text<<"AUSROUTE";
        bstyle_text<<"AUSREDIRECT";
        bstyle_text<<"ISBNX";
        bstyle_text<<"RM4SCC";
        bstyle_text<<"DATAMATRIX";
        bstyle_text<<"ITF14";
        bstyle_text<<"CODABLOCKF";
        bstyle_text<<"NVE18";
        bstyle_text<<"KOREAPOST";
        bstyle_text<<"RSS14STACK";
        bstyle_text<<"RSS14STACK_OMNI";
        bstyle_text<<"RSS_EXPSTACK";
        bstyle_text<<"PLANET";
        bstyle_text<<"MICROPDF417";
        bstyle_text<<"ONECODE";
        bstyle_text<<"PLESSEY";
        bstyle_text<<"KIX";
        bstyle_text<<"AZTEC";
        bstyle_text<<"DAFT";
        bstyle_text<<"ITALYPOST";
        bstyle_text<<"DPD";
        bstyle_text<<"MICROQR";
        bstyle_text<<"TELEPEN_NUM";
        bstyle_text<<"CODE32";
        bstyle_text<<"EANX_CC";
        bstyle_text<<"EAN128_CC";
        bstyle_text<<"RSS14_CC";
        bstyle_text<<"RSS_LTD_CC";
        bstyle_text<<"RSS_EXP_CC";
        bstyle_text<<"UPCA_CC";
        bstyle_text<<"UPCE_CC";
        bstyle_text<<"RSS14STACK_CC";
        bstyle_text<<"RSS14_OMNI_CC";
        bstyle_text<<"RSS_EXPSTACK_CC";
    return bstyle_text;
}

BarCode::FrameTypePairList BarCode::getFrameTypeList()
{
    FrameTypePairList list;
    const QMetaObject &mo = staticMetaObject;
    for (int i=0; i < mo.enumerator(1).keyCount(); i++) {
        QPair<FrameTypes, QString> pair;
        pair.first = (FrameTypes)mo.enumerator(1).value(i);
        pair.second = mo.enumerator(1).key(i);
        list.append(pair);
    }
    return list;
}

QString BarCode::getFrameTypeName(FrameTypes type)
{
    const QMetaObject &mo = staticMetaObject;
    for (int i=0; i < mo.enumerator(1).keyCount(); i++)
        if (mo.enumerator(1).value(i) == type)
            return mo.enumerator(1).key(i);

    return QString();
}

QString BarCode::getValue()
{
    return m_value;
}

void BarCode::setValue(QString value)
{
    m_value = value;
    this->repaint();
}

BarCode::~BarCode()
{
    #ifndef NO_BARCODE
        delete bc;
    #endif
}

void BarCode::setProperties()
{
    this->setProperty("BarcodeTypes",m_BarcodeType);
    this->setProperty("FrameTypes",m_FrameType);
}

//Restore fields from properties
void BarCode::setParamFromProperties()
{
    m_BarcodeType = (BarcodeTypes)this->property("BarcodeTypes").toInt();
    m_FrameType = (FrameTypes)this->property("FrameTypes").toInt();
}

QDataStream &operator<<(QDataStream &stream, const BarCode &obj)
{
    for (int i=0; i<obj.metaObject()->propertyCount(); ++i)
        if (obj.metaObject()->property(i).isStored())
            stream << obj.metaObject()->property(i).read(&obj);

    for (auto &byteArray : obj.dynamicPropertyNames())
        stream << obj.property(byteArray);

    return stream;
}

QDataStream &operator>>(QDataStream &stream, BarCode &obj)
{
    QVariant var;
    for (int i=0; i<obj.metaObject()->propertyCount(); ++i) {
        if (obj.metaObject()->property(i).isStored()) {
            stream >> var;
            if (!var.isNull())
                obj.metaObject()->property(i).write(&obj, var);
        }
    }
    obj.setProperties();

    for (auto &byteArray : obj.dynamicPropertyNames()) {
        stream >> var;
        obj.setProperty(byteArray, QVariant(var));
    }
    obj.setParamFromProperties();

    return stream;
}
