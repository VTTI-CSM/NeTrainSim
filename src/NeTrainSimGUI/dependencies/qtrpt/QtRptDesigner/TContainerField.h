/*
Name: QtRpt
Version: 1.5.5
Web-site: http://www.qtrpt.tk
Programmer: Aleksey Osipov
E-mail: aliks-os@ukr.net
Web-site: http://www.aliks-os.tk

Copyright 2012-2015 Aleksey Osipov

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

#ifndef TCONTAINERFIELD_H
#define TCONTAINERFIELD_H

#include "RptContainer.h"
#include "EditFldDlg.h"
#include <chart.h>
#include <Barcode.h>
#include <RptCrossTabObject.h>

class TContainerField : public RptContainer {
    Q_OBJECT    
    Q_PROPERTY(QString text READ getText WRITE setText)
    Q_PROPERTY(int fontSize READ getFontSize WRITE setFontSize)
    Q_PROPERTY(bool autoHeight READ getAutoHeight WRITE setAutoHeight)
    Q_PROPERTY(QString imgFormat READ getImgFormat WRITE setImgFromat)
    Q_PROPERTY(bool ignoreAspectRatio READ getIgnoreAspectRatio WRITE setIgnoreAspectRatio)
    Q_PROPERTY(bool RTL READ getTextDirection WRITE setTextDirection)
    Q_PROPERTY(QString formatString READ getFormatString WRITE setFormatString)
    Q_PROPERTY(QString Highlighting READ getHighlighting WRITE setHighlighting)
    Q_PROPERTY(QPixmap image READ getImage WRITE setImage)
    Q_PROPERTY(QFont font READ getFont WRITE setFont)
    Q_PROPERTY(Qt::Alignment Alignment READ getAligment WRITE setAligment)
    Q_PROPERTY(bool textWrap READ getTextWrap WRITE setTextWrap)

public:
    TContainerField(QWidget *parent, QPoint p, QWidget *cWidget = 0);
    ~TContainerField();
    TContainerField *clone();
    void setText(QString text);
    QString getText();
    void setImage(QPixmap p);
    QPixmap getImage();
    void setMenu(QMenu *menu_);
    void setType(FieldType value, QDomDocument *xmlDoc);
    Chart *getChart();
    void setFontSize(int value);
    int getFontSize();
    QString getImgFormat();
    void setImgFromat(QString value);
    bool getIgnoreAspectRatio();
    void setIgnoreAspectRatio(bool value);
    BarCode::BarcodeTypes getBarcodeType();
    void setBarcodeType(BarCode::BarcodeTypes value);
    BarCode::FrameTypes getBarcodeFrameType();
    void setBarcodeFrameType(BarCode::FrameTypes value);
    int getBarcodeHeight();
    void setBarcodeHeight(int value);
    bool getAutoHeight() {return m_autoHeight;}
    void setAutoHeight(bool value) {m_autoHeight = value;}
    QString getHighlighting() {return m_highlighting;}
    void setHighlighting(QString value) {m_highlighting = value;}
    QString getFormatString() {return m_formatString;}
    void setFormatString(QString value) {m_formatString = value;}
    void loadParamFromXML(QDomElement e);
    QDomElement saveParamToXML(QDomDocument *xmlDoc);
    void setProperties();
    void setParamFromProperties();
    void setTextDirection(bool value) {m_textDirection = value;}
    bool getTextDirection() {return m_textDirection;}
    void setTextWrap(bool value);
    bool getTextWrap() {return m_textWrap;}
    RptCrossTabObject *getCrossTab();

    friend QDataStream &operator<<(QDataStream &stream, const TContainerField &obj);
    friend QDataStream &operator>>(QDataStream &stream, TContainerField &obj);

protected:    
    void mouseDoubleClickEvent ( QMouseEvent * event );
    void paintEvent( QPaintEvent * event);
    void resizeEvent(QResizeEvent *e);

private:
    QString m_highlighting;
    QLabel *m_label;
    Chart *m_chart;
    qreal radius;
    qreal fontSizeNoScale;
    QString m_imgFormat;
    bool m_ignoreAspectRatio;
    QPixmap m_pixmap;
    QDomDocument *m_xmlDoc;
    BarCode *m_barcode;
    RptCrossTabObject *m_crossTab;
    bool m_autoHeight;
    QString m_formatString;
    void setFont(QFont font) {m_label->setFont(font);}
    QFont getFont() {return m_label->font();}
    void setAligment(Qt::Alignment aligment) {m_label->setAlignment(aligment);}
    Qt::Alignment getAligment() {return m_label->alignment();}
    bool m_textDirection;
    bool m_textWrap;

signals:

public slots:
    void edit();
    void moveForward();
    void moveBack();

private slots:

};

#endif // TCONTAINERFIELD_H
