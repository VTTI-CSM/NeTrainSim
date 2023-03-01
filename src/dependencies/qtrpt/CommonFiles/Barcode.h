#ifndef BARCODE_H
#define BARCODE_H

#include <QWidget>
#include <QMetaEnum>
#ifndef NO_BARCODE
    #include <qzint.h>
#endif
#include <QPainter>

class BarCode : public QWidget
{
    Q_OBJECT
    Q_ENUMS(BarcodeTypes)
    Q_ENUMS(FrameTypes)
    Q_PROPERTY(QString value READ getValue WRITE setValue)

public:
    enum BarcodeTypes {
        CODE11		=1,
        C25MATRIX	=2,
        C25INTER	=3,
        C25IATA		=4,
        C25LOGIC	=6,
        C25IND		=7,
        CODE39		=8,
        EXCODE39	=9,
        EANX		=13,
        EAN128		=16,
        CODABAR		=18,
        CODE128		=20,
        DPLEIT		=21,
        DPIDENT		=22,
        CODE16K		=23,
        CODE93		=25,
        FLAT		=28,
        RSS14		=29,
        RSS_LTD		=30,
        RSS_EXP		=31,
        TELEPEN		=32,
        UPCA		=34,
        UPCE		=37,
        POSTNET		=40,
        MSI_PLESSEY	=47,
        FIM		=49,
        LOGMARS		=50,
        PHARMA		=51,
        PZN		=52,
        PHARMA_TWO	=53,
        PDF417		=55,
        PDF417TRUNC	=56,
        MAXICODE	=57,
        QRCODE		=58,
        CODE128B	=60,
        AUSPOST		=63,
        AUSREPLY	=66,
        AUSROUTE	=67,
        AUSREDIRECT	=68,
        ISBNX		=69,
        RM4SCC		=70,
        DATAMATRIX	=71,
        ITF14		=72,
        CODABLOCKF	=74,
        NVE18		=75,
        KOREAPOST	=77,
        RSS14STACK	=79,
        RSS14STACK_OMNI	=80,
        RSS_EXPSTACK	=81,
        PLANET		=82,
        MICROPDF417	=84,
        ONECODE		=85,
        PLESSEY		=86,

        /* TBarcodeItem 8 codes */
        KIX		=90,
        AZTEC		=92,
        DAFT		=93,
        ITALYPOST	=94,
        DPD		=96,
        MICROQR		=97,

        /* Zint specific */
        TELEPEN_NUM	=128,
        CODE32		=129,
        EANX_CC		=130,
        EAN128_CC	=131,
        RSS14_CC	=132,
        RSS_LTD_CC	=133,
        RSS_EXP_CC	=134,
        UPCA_CC		=135,
        UPCE_CC		=136,
        RSS14STACK_CC	=137,
        RSS14_OMNI_CC	=138,
        RSS_EXPSTACK_CC	=139
    };
    enum FrameTypes {
        NO_BORDER=0, BIND=2, BOX=4
    };

public:
    explicit BarCode(QWidget *parent = 0);
    ~BarCode();
    void drawBarcode(QPainter *painter, qreal x, qreal y, qreal width, qreal height);
    QString getValue();
    void setBarcodeType(BarcodeTypes value);
    BarcodeTypes getBarcodeType() {return m_BarcodeType;}
    void setFrameType(FrameTypes value);
    FrameTypes getFrameType() {return m_FrameType;}
    void setHeight(int value);
    int getHeight() {return m_height;}
    typedef QList<QPair<BarcodeTypes, QString> > BarcodeTypePairList;
    typedef QList<QPair<FrameTypes, QString> > FrameTypePairList;
    static BarcodeTypePairList getTypeList();
    static QString getTypeName(BarcodeTypes type);
    static FrameTypePairList getFrameTypeList();
    static QString getFrameTypeName(FrameTypes type);

    void setProperties();
    friend QDataStream &operator<<(QDataStream &stream, const BarCode &obj);
    friend QDataStream &operator>>(QDataStream &stream, BarCode &obj);

protected:
    void paintEvent(QPaintEvent * event);
    void setParamFromProperties();

private:
    #ifndef NO_BARCODE
        Zint::QZint *bc;
    #endif
    QString m_value;
    BarcodeTypes m_BarcodeType;
    FrameTypes m_FrameType;
    int m_height;
    const static QStringList getTypeNameList();

signals:

public slots:
    void setValue(QString value);

};

using SPtrBarCode = QSharedPointer<BarCode>;

#endif // BARCODE_H
