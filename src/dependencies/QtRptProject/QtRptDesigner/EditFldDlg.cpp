/*
Name: QtRpt
Version: 2.0.2
Web-site: https://qtrpt.sourceforge.io
Programmer: Aleksey Osipov
E-mail: aliks-os@ukr.net
Web-site: http://www.aliks-os.tk

Copyright 2012-2018 Aleksey Osipov

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

#include "mainwindow.h"
#include "EditFldDlg.h"
#include "ui_EditFldDlg.h"
#include "tableDelegates.h"
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>

EditFldDlg::EditFldDlg(QWidget *parent)
: QDialog(parent), ui(new Ui::EditFldDlg)
{
    ui->setupUi(this);

    QObject::connect(ui->btnLoadImage, SIGNAL(clicked()), this, SLOT(loadImage()));
    QObject::connect(ui->btnSaveImage, SIGNAL(clicked()), this, SLOT(saveImage()));
    QObject::connect(ui->btnAddVariable, SIGNAL(clicked()), this, SLOT(openProperty()));
    QObject::connect(ui->btnFormatting, SIGNAL(clicked()), this, SLOT(openProperty()));
    QObject::connect(ui->btnAddFunction, SIGNAL(clicked()), this, SLOT(openProperty()));
    QObject::connect(ui->rdPrinting, SIGNAL(toggled(bool)), this, SLOT(conditionalToggled(bool)));
    QObject::connect(ui->rdTransparent, SIGNAL(toggled(bool)), this, SLOT(backGroundToggled(bool)));
    QObject::connect(ui->edtCondition, SIGNAL(textEdited(const QString&)), this, SLOT(conditionChanged(const QString&)));
    QObject::connect(ui->btnColorB, SIGNAL(clicked()), this, SLOT(chooseColor()));
    QObject::connect(ui->btnColorF, SIGNAL(clicked()), this, SLOT(chooseColor()));
    QObject::connect(ui->btnColorTotal, SIGNAL(clicked()), this, SLOT(chooseColor()));
    QObject::connect(ui->btnColorHeader, SIGNAL(clicked()), this, SLOT(chooseColor()));
    QObject::connect(ui->btnColorSeries, SIGNAL(clicked()), this, SLOT(chooseColor()));
    QObject::connect(ui->btnColorTitle, SIGNAL(clicked()), this, SLOT(chooseColor()));
    QObject::connect(ui->btnColorLegend, SIGNAL(clicked()), this, SLOT(chooseColor()));
    QObject::connect(ui->btnColorBackground, SIGNAL(clicked()), this, SLOT(chooseColor()));
    QObject::connect(ui->chkBold, SIGNAL(clicked()), this, SLOT(encodeHighLightingString()));
    QObject::connect(ui->chkItalic, SIGNAL(clicked()), this, SLOT(encodeHighLightingString()));
    QObject::connect(ui->chkUnderline, SIGNAL(clicked()), this, SLOT(encodeHighLightingString()));
    QObject::connect(ui->chkStrikeout, SIGNAL(clicked()), this, SLOT(encodeHighLightingString()));


    QObject::connect(ui->btnAddValue, SIGNAL(clicked()), this, SLOT(seriesValue()));
    QObject::connect(ui->btnRemoveValue, SIGNAL(clicked()), this, SLOT(seriesValue()));
    QObject::connect(ui->btnAddSeries, SIGNAL(clicked()), this, SLOT(addSeries()));
    QObject::connect(ui->btnRemoveSeries, SIGNAL(clicked()), this, SLOT(removeSeries()));
    QObject::connect(ui->tableSeries, SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
                     this, SLOT(seriesDoubleClicked(QTableWidgetItem*)));
    QObject::connect(ui->btnTitleFont, SIGNAL(clicked()), this, SLOT(fontSelect()));
    QObject::connect(ui->btnLegendFont, SIGNAL(clicked()), this, SLOT(fontSelect()));
}

void EditFldDlg::fontSelect()
{
    if (sender() == ui->btnTitleFont)
        ui->fntTitle->setCurrentFont(QFontDialog::getFont(0, ui->fntTitle->currentFont()));
    if (sender() == ui->btnLegendFont)
        ui->fntLegend->setCurrentFont(QFontDialog::getFont(0, ui->fntLegend->currentFont()));
}

void EditFldDlg::chooseColor()
{
    QColor color;
    QLabel *label = nullptr;

    if (sender() == ui->btnColorF)
        label = ui->lblColorF;
    if (sender() == ui->btnColorB)
        label = ui->lblColorB;
    if (sender() == ui->btnColorTotal)
        label = ui->lblColorTotal;
    if (sender() == ui->btnColorHeader)
        label = ui->lblColorHeader;
    if (sender() == ui->btnColorSeries)
        label = ui->lblColorSeries;
    if (sender() == ui->btnColorTitle)
        label = ui->lblColorTitle;
    if (sender() == ui->btnColorLegend)
        label = ui->lblColorLegend;
    if (sender() == ui->btnColorBackground)
        label = ui->lblColorBackground;

    if (label != nullptr)
        color = colorFromString(label->styleSheet());


    QScopedPointer<QColorDialog> dlg(new QColorDialog(color, this));
    if (dlg->exec() == QDialog::Accepted)
        color = dlg->selectedColor();
    else
        return;

    QString strColor = colorToString(color);
    if (label != nullptr)
        label->setStyleSheet("QLabel {background-color: "+strColor+"}");

    if (sender() != ui->btnColorSeries)
        encodeHighLightingString();
}

void EditFldDlg::conditionChanged(const QString &text)
{
    if (ui->rdPrinting->isChecked())
        m_cond_printing = text;
    if (ui->rdHighlighting->isChecked())
        encodeHighLightingString();
}

// Switching between Printing and Highlighting
void EditFldDlg::conditionalToggled(bool value)
{
    ui->grpBackground->setEnabled(!value);
    ui->grpFont->setEnabled(!value);

    if (value) {  // Show printting condition
        encodeHighLightingString();
        ui->edtCondition->setText(m_cond_printing);
    } else {  // Show highlighting condtion
        m_cond_printing = ui->edtCondition->text();
        decodeHighLightingString();
        ui->edtCondition->setText(m_cond_higlighting.section(";",0,0));
    }
    backGroundToggled(!false);
}

void EditFldDlg::decodeHighLightingString()
{
    for (const auto &str : m_cond_higlighting.split(";")) {
        if (str.contains("bold"))
            ui->chkBold->setChecked(true);
        if (str.contains("italic"))
            ui->chkItalic->setChecked(true);
        if (str.contains("underline"))
            ui->chkUnderline->setChecked(true);
        if (str.contains("strikeout"))
            ui->chkStrikeout->setChecked(true);
        if (str.contains("fontColor")) {
            int start = str.indexOf("rgba(",0,Qt::CaseInsensitive);
            ui->lblColorF->setStyleSheet("QLabel {background-color: "+str.mid(start)+"}");
        }
        if (str.contains("backgroundColor")) {
            int start = str.indexOf("rgba(",0,Qt::CaseInsensitive);
            ui->lblColorB->setStyleSheet("QLabel {background-color: "+str.mid(start)+"}");
            ui->rdOther->setChecked(true);
            backGroundToggled(false);
        }
    }
}

void EditFldDlg::encodeHighLightingString()
{
    if (ui->edtCondition->text().isEmpty())
        return;
    if (ui->rdPrinting->isChecked())
        return;
    m_cond_higlighting.clear();

    QString tmpStr = ui->edtCondition->text();
    m_cond_higlighting = tmpStr+";"; //First item in section is always a condition

    if (ui->chkBold->isChecked())
        m_cond_higlighting  += "bold=?1:0;";
    if (ui->chkItalic->isChecked())
        m_cond_higlighting += "italic=?1:0;";
    if (ui->chkUnderline->isChecked())
        m_cond_higlighting += "underline=?1:0;";
    if (ui->chkStrikeout->isChecked())
        m_cond_higlighting += "strikeout=?1:0;";

    int startF = ui->lblColorF->styleSheet().indexOf("rgba(",0,Qt::CaseInsensitive);
    int endF = ui->lblColorF->styleSheet().indexOf(")",Qt::CaseInsensitive)+1;
    int startB = ui->lblColorB->styleSheet().indexOf("rgba(",0,Qt::CaseInsensitive);
    int endB = ui->lblColorB->styleSheet().indexOf(")",Qt::CaseInsensitive)+1;

    QString strColorF = ui->lblColorF->styleSheet().mid(startF,endF-startF);
    QString strColorB = ui->lblColorB->styleSheet().mid(startB,endB-startB);

    if (!strColorF.isEmpty() && strColorF != "255,255,255,255" && strColorF != "255,255,255,0")
        m_cond_higlighting += "fontColor=?"+strColorF+";";

    if (ui->rdOther->isChecked())
        if (!strColorB.isEmpty() && strColorB != "255,255,255,255" && strColorB != "255,255,255,0")
            m_cond_higlighting += "backgroundColor=?"+strColorB+";";
}

void EditFldDlg::backGroundToggled(bool value)
{
    ui->lblColorB->setEnabled(!value);
    ui->btnColorB->setEnabled(!value);
    if (ui->rdHighlighting->isChecked())
        encodeHighLightingString();
}

void EditFldDlg::openProperty()
{
    QScopedPointer<FldPropertyDlg> dlg(new FldPropertyDlg(this));

    if (sender() == ui->btnAddVariable) {
        QString str = dlg->showThis(0, nullptr);
        ui->textEdit->insertPlainText(str);
    }
    if (sender() == ui->btnAddFunction) {
        QString str = dlg->showThis(3, nullptr);
        ui->textEdit->insertPlainText(str);
    }
    if (sender() == ui->btnFormatting) {
         QString str = dlg->showThis(2, m_cont);
         m_cont->setFormatString(str);
    }
}

void EditFldDlg::textDirection()
{
    QTextOption topt = ui->textEdit->document()->defaultTextOption();
    if (ui->btnTextDirection->isChecked())
        topt.setTextDirection(Qt::RightToLeft);
    else
        topt.setTextDirection(Qt::LeftToRight);
    ui->textEdit->document()->setDefaultTextOption(topt);
}

int EditFldDlg::showText(QGraphicsItem *gItem)
{
    auto cont = qgraphicsitem_cast<GraphicsBox*>(gItem);
    ui->textEdit->setPlainText(cont->getText());
    ui->stackedWidget->setCurrentIndex(0);
    ui->textEdit->setFocus();
    m_cont = cont;

    ui->btnTextDirection->setEnabled(true);
    QObject::connect(ui->btnTextDirection, SIGNAL(clicked()), this, SLOT(textDirection()));

    if (cont->getTextDirection())
        ui->btnTextDirection->click();

    switch (cont->getFieldType()) {
    case TextImage:
        ui->radioButtonTextImage->setChecked(true);
        boolImage = true;
        break;
    case DatabaseImage:
        ui->radioButtonDatabaseImage->setChecked(true);
        boolImage = true;
        break;
    default:
        ui->radioButtonText->setChecked(true);
        boolImage = false;
        break;
    }

    ui->lblAttention->setVisible(boolImage);
    ui->rdPrinting->setChecked(true);

    if (cont->getPrinting().size() > 1) {
        m_cond_printing = cont->getPrinting().replace("?1:0","");
        ui->edtCondition->setText(m_cond_printing);
    }
    if (cont->getHighlighting().size() > 0) {
        m_cond_higlighting = cont->getHighlighting();
        decodeHighLightingString();
    }

    // Creating Completer
    auto completer = new QCompleter(this);
    QAbstractItemModel *model = nullptr;

    auto mw = MainWindow::instance();
    model = new QStringListModel(mw->getWords(), completer);

    completer->setModel(model);
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);
    ui->textEdit->setCompleter(completer);
    //************************

    if (this->exec()) {
        if (ui->radioButtonTextImage->isChecked())
            cont->setFieldType(TextImage);
        else if (ui->radioButtonDatabaseImage->isChecked())
            cont->setFieldType(DatabaseImage);
        else
            cont->setFieldType(Text);

        QString plainTextEditContents = ui->textEdit->toPlainText();
        QStringList lines = plainTextEditContents.split("\n");

        if (lines.count() > 1 && lines.last().trimmed().isEmpty()) {
            QMessageBox::StandardButton reply;            
            reply = QMessageBox::question(this, tr("Empty line"),tr("The field contains empty line at the end.\nRemove it?"),
                                             QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                plainTextEditContents.clear();
                //Remove last empty lines
                int i = lines.count()-1;
                while (i > 0) {
                    if (lines.last().trimmed().isEmpty() && i==lines.count()-1)
                        lines.removeAt(lines.count()-1);
                    else
                        i--;
                }
                //join all in one string
                for (int i = 0; i < lines.size(); ++i) {
                    plainTextEditContents += lines.at(i);
                    if (i != lines.size()-1)
                        plainTextEditContents += "\n";
                }
            }
        }

        if (cont->getTextDirection() != ui->btnTextDirection->isChecked()) {
            if (ui->btnTextDirection->isChecked())
                cont->setAlignment(Qt::AlignRight);
            else
                cont->setAlignment(Qt::AlignLeft);
        }
        cont->setTextDirection(ui->btnTextDirection->isChecked());

        cont->setText(plainTextEditContents);

        if (m_cond_printing.size() > 0) {
            cont->setPrinting( m_cond_printing+"?1:0" );
        } else {
            if (cont->getPrinting().size() > 1)  //If previous was a Formula, now just a Visible
                cont->setPrinting("1");
        }

        if (m_cond_higlighting.size() > 0)
            cont->setHighlighting(m_cond_higlighting);
        else
            cont->setHighlighting("");

        return QDialog::Accepted;
    } else {
        return QDialog::Rejected;
    }
}

int EditFldDlg::showTextRich(QGraphicsItem *gItem)
{
    auto cont = qgraphicsitem_cast<GraphicsBox*>(gItem);
    ui->textEditRich->textEdit->setHtml(cont->getText());
    ui->stackedWidget->setCurrentIndex(4);
    ui->textEdit->setFocus();
    m_cont = cont;

    // Creating Completer
    auto completer = new QCompleter(this);
    QAbstractItemModel *model = nullptr;

    auto mw = MainWindow::instance();
    model = new QStringListModel(mw->getWords(), completer);

    completer->setModel(model);
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);
    ui->textEditRich->setCompleter(completer);
    //************************

    if (this->exec()) {
        QString plainTextEditContents = ui->textEditRich->textEdit->toHtml();
        auto lines = plainTextEditContents.split("\n");

        if (lines.count() > 1 && lines.last().trimmed().isEmpty()) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("Empty line"),tr("The field contains empty line at the end.\nRemove it?"),
                                          QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                plainTextEditContents.clear();
                // Remove last empty lines
                int i = lines.count()-1;
                while (i > 0) {
                    if (lines.last().trimmed().isEmpty() && i==lines.count()-1)
                        lines.removeAt(lines.count()-1);
                    else
                        i--;
                }

                // join all in one string
                for (int i = 0; i < lines.size(); ++i) {
                    plainTextEditContents += lines.at(i);
                    if (i != lines.size()-1)
                        plainTextEditContents += "\n";
                }
            }
        }

        cont->setText(plainTextEditContents);

        return QDialog::Accepted;
    }
    else
        return QDialog::Rejected;
}

void EditFldDlg::seriesDoubleClicked(QTableWidgetItem *item)
{
    auto row = item->row();
    auto series = m_chart->series().at(0);

    auto dlg = new EditFldDlg(this);
    dlg->showSeries(series, row);


    if (dlg->result() == QDialog::Accepted) {
        if (ui->cbChartType->currentData().toInt() == QAbstractSeries::SeriesTypeLine) {
            m_chart->removeSeries(series);
            m_chart->addSeries(series);
            m_chart->createDefaultAxes();

            fillSeriesTbl();
        }
    }

    delete dlg;
}

int EditFldDlg::showSeries(QAbstractSeries *abstrSeries, int barSetNo)
{
    ui->stackedWidget->setCurrentIndex(6);

    switch(abstrSeries->type()) {
    case QAbstractSeries::SeriesTypeStackedBar:
    case QAbstractSeries::SeriesTypeBar: {
        QBarSet *barSet = nullptr;

        if (abstrSeries->type() == QAbstractSeries::SeriesTypeBar) {
            auto series = qobject_cast<QBarSeries*>(abstrSeries);
            barSet = series->barSets().at(barSetNo);
        } else {
            auto series = qobject_cast<QStackedBarSeries*>(abstrSeries);
            barSet = series->barSets().at(barSetNo);
        }

        this->setWindowTitle("Bar property");
        ui->edtSeriesName->setText(barSet->label());
        ui->edtSeriesDS->setText(barSet->property("graphDS").toString());

        ui->edtPieValue->setVisible(false);
        ui->lblPieValue->setVisible(false);
        ui->tblSeriesValues->setVisible(true);
        ui->btnAddValue->setVisible(true);
        ui->btnRemoveValue->setVisible(true);
        ui->chkSliceCaption->setVisible(false);
        ui->chkSliceExploded->setVisible(false);

        QString strColor = colorToString(barSet->color());
        ui->lblColorSeries->setStyleSheet("QLabel {background-color: "+strColor+"}");

        ui->tblSeriesValues->setRowCount(barSet->count());

        for (int row = 0; row < barSet->count(); row++) {
            auto value = barSet->at(row);
            auto newItem = new QTableWidgetItem(QString::number(value));
            ui->tblSeriesValues->setItem(row, 0, newItem);
        }
        break;
    }
    case QAbstractSeries::SeriesTypePie: {
        auto series = qobject_cast<QPieSeries*>(abstrSeries);
        auto slice = series->slices().at(barSetNo);

        ui->chkSliceCaption->setChecked(slice->isLabelVisible());
        ui->chkSliceExploded->setChecked(slice->isExploded());


        ui->edtSeriesDS->setText(abstrSeries->property("graphDS").toString());
        ui->edtSeriesName->setText(slice->label());

        ui->edtPieValue->setVisible(true);
        ui->lblPieValue->setVisible(true);
        ui->tblSeriesValues->setVisible(false);
        ui->btnAddValue->setVisible(false);
        ui->btnRemoveValue->setVisible(false);
        ui->lblStaticValues->setVisible(false);

        this->setWindowTitle("Pie property");

        ui->lblSeriesDS->setText("Pie data source");
        ui->lblSeriesName->setText("Pie name");
        ui->lblSeriesColor->setText("Pie color");

        QString strColor = colorToString(slice->color());
        ui->lblColorSeries->setStyleSheet("QLabel {background-color: "+strColor+"}");

        ui->tblSeriesValues->setRowCount(1);

        ui->edtPieValue->setText(QString::number(slice->value()));

        break;
    }
    case QAbstractSeries::SeriesTypeLine: {
        ui->edtSeriesDS->setText(abstrSeries->property("graphDS").toString());
        ui->edtSeriesName->setText(abstrSeries->name());
        auto series = qobject_cast<QLineSeries*>(abstrSeries);

        ui->edtPieValue->setVisible(false);
        ui->lblPieValue->setVisible(false);
        ui->tblSeriesValues->setVisible(true);
        ui->btnAddValue->setVisible(true);
        ui->btnRemoveValue->setVisible(true);
        ui->chkSliceCaption->setVisible(false);
        ui->chkSliceExploded->setVisible(false);

        this->setWindowTitle("Line property");

        QString strColor = colorToString(series->color());
        ui->lblColorSeries->setStyleSheet("QLabel {background-color: "+strColor+"}");

        ui->tblSeriesValues->setColumnCount(1);
        ui->tblSeriesValues->setRowCount(series->count());

        for (int row = 0; row < series->count(); row++) {
            auto point = series->at(row);
            auto newItem = new QTableWidgetItem(QString::number(point.y()));
            ui->tblSeriesValues->setItem(row, 0, newItem);
        }
        break;
    }
    default: qDebug() << "Type is not defined";
    }

    if (this->exec()) {
        switch(abstrSeries->type()) {
        case QAbstractSeries::SeriesTypeStackedBar:
        case QAbstractSeries::SeriesTypeBar: {
            QBarSet *barSet = nullptr;

            if (abstrSeries->type() == QAbstractSeries::SeriesTypeBar) {
                auto series = qobject_cast<QBarSeries*>(abstrSeries);
                barSet = series->barSets().at(barSetNo);
            } else {
                auto series = qobject_cast<QStackedBarSeries*>(abstrSeries);
                barSet = series->barSets().at(barSetNo);
            }

            barSet->remove(0, barSet->count());
            barSet->setProperty("graphDS", ui->edtSeriesDS->text());

            for (int row = 0; row < ui->tblSeriesValues->rowCount(); row++) {
                auto item = ui->tblSeriesValues->item(row, 0);
                barSet->append(item->text().toFloat());
            }

            QColor color = colorFromString(ui->lblColorSeries->styleSheet());
            barSet->setColor(color);
            barSet->setLabel(ui->edtSeriesName->text());
            break;
        }
        case QAbstractSeries::SeriesTypePie: {
            auto series = qobject_cast<QPieSeries*>(abstrSeries);
            auto slice = series->slices().at(barSetNo);

            series->remove(slice);

            series->setProperty("graphDS", ui->edtSeriesDS->text());

            slice = new QPieSlice(ui->edtSeriesName->text(), ui->edtPieValue->text().toFloat(), series);
            slice->setExploded(ui->chkSliceExploded->isChecked());
            slice->setLabelVisible(ui->chkSliceCaption->isChecked());

            series->insert(barSetNo, slice);

            QColor color = colorFromString(ui->lblColorSeries->styleSheet());
            slice->setColor(color);
            slice->setLabel(ui->edtSeriesName->text());
            break;
        }
        case QAbstractSeries::SeriesTypeLine: {
            auto series = qobject_cast<QLineSeries*>(abstrSeries);
            series->removePoints(0, series->count());

            for (int row = 0; row < ui->tblSeriesValues->rowCount(); row++) {
                auto item = ui->tblSeriesValues->item(row, 0);
                series->append(row, item->text().toDouble());
            }

            QColor color = colorFromString(ui->lblColorSeries->styleSheet());
            series->setColor(color);
            series->setName(ui->edtSeriesName->text());
            series->setProperty("graphDS", ui->edtSeriesDS->text());
            break;
        }
        default: qDebug() << "Type is not defined";
        }

        return QDialog::Accepted;
    } else
        return QDialog::Rejected;
}

int EditFldDlg::showImage(QGraphicsItem *gItem)
{
    auto cont = qgraphicsitem_cast<GraphicsBox*>(gItem);

    ui->stackedWidget->setCurrentIndex(1);
    ui->label->setPixmap(cont->getImage());
    QObject::connect(ui->chkIgnoreAspectRatio, SIGNAL(toggled(bool)), this, SLOT(setScaledContents(bool)));

    ui->chkIgnoreAspectRatio->setChecked(cont->getIgnoreAspectRatio());
    m_imgFormat = cont->getImgFormat();

    if (this->exec()) {
        cont->setIgnoreAspectRatio(ui->chkIgnoreAspectRatio->isChecked());
        #if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
            cont->setImage(ui->label->pixmap(Qt::ReturnByValue));
        #else
            cont->setImage(*ui->label->pixmap());
        #endif
        cont->setImgFromat(m_imgFormat);

        return QDialog::Accepted;
    } else {
        return QDialog::Rejected;
    }
}

int EditFldDlg::showBarcode(QGraphicsItem *gItem)
{
    auto cont = qgraphicsitem_cast<GraphicsBox*>(gItem);
    ui->stackedWidget->setCurrentIndex(3);
    QObject::connect(ui->edtValue, SIGNAL(textChanged(QString)), ui->wBarcode, SLOT(setValue(QString)));
    QObject::connect(ui->bstyle, SIGNAL(activated(int)), SLOT(update_preview()));
    QObject::connect(ui->cbFrameType, SIGNAL(activated(int)), SLOT(update_preview()));
    QObject::connect(ui->spnHeight, SIGNAL(valueChanged(int)), SLOT(update_preview()));
    ui->edtValue->setText(cont->getText());

    BarCode::BarcodeTypePairList list1 = BarCode::getTypeList();
    for (int i = 0; i < list1.size(); i++) {
        ui->bstyle->addItem(list1.at(i).second,list1.at(i).first);

        if (list1.at(i).first == cont->getBarcodeType() )
            ui->bstyle->setCurrentIndex(i);
    }

    BarCode::FrameTypePairList list2 = BarCode::getFrameTypeList();
    for (int i = 0; i < list2.size(); i++) {
        ui->cbFrameType->addItem(list2.at(i).second,list2.at(i).first);

        if (list2.at(i).first == cont->getBarcodeFrameType() )
            ui->cbFrameType->setCurrentIndex(i);
    }
    ui->spnHeight->setValue(cont->getBarcodeHeight());

    if (this->exec()) {
        cont->setText(ui->edtValue->text());
        cont->setBarcodeType((BarCode::BarcodeTypes)ui->wBarcode->metaObject()->enumerator(0).value(ui->bstyle->currentIndex()));
        cont->setBarcodeFrameType((BarCode::FrameTypes)ui->wBarcode->metaObject()->enumerator(1).value(ui->cbFrameType->currentIndex()));
        cont->setBarcodeHeight(ui->spnHeight->value());

        return QDialog::Accepted;
    } else {
        return QDialog::Rejected;
    }
}

int EditFldDlg::showCrosstab(QGraphicsItem *gItem)
{
    auto cont = qgraphicsitem_cast<GraphicsBox*>(gItem);
    auto m_crossTab = cont->getCrossTab();

    if (m_crossTab == nullptr)
        return QDialog::Rejected;

    ui->stackedWidget->setCurrentIndex(5);
    if (m_crossTab->isTotalByRowVisible())
        ui->spnColCount->setValue(m_crossTab->colCount()-1);
    else
        ui->spnColCount->setValue(m_crossTab->colCount());



    auto d = new SpinDelegate(0,
                              500,
                              1,
                              2,
                              ui->colTable);
    ui->colTable->setItemDelegateForColumn(2, d);
    for (const auto &col : m_crossTab->columns) {
        ui->colTable->setRowCount(ui->colTable->rowCount() + 1);

        auto newItem = new QTableWidgetItem(col.caption);
        ui->colTable->setItem(ui->colTable->rowCount() - 1, 0, newItem);

        newItem = new QTableWidgetItem(col.value);
        ui->colTable->setItem(ui->colTable->rowCount() - 1, 1, newItem);

        newItem = new QTableWidgetItem(QString::number(col.width));
        ui->colTable->setItem(ui->colTable->rowCount() - 1, 2, newItem);
    }

    ui->colTable->setRowCount(ui->spnColCount->value());

    ui->spnRowHeight->setValue(m_crossTab->rowHeight());
    ui->chkRowTotal->setChecked(m_crossTab->isTotalByRowVisible());
    ui->chkColTotal->setChecked(m_crossTab->isTotalByColumnVisible());
    ui->chkColSubTotal->setChecked(m_crossTab->isSubTotalVisible());
    ui->chckHeader->setChecked(m_crossTab->isHeaderVisible());

    QColor colorTotal = m_crossTab->totalBackgroundColor;
    QColor colorHeader = m_crossTab->headerBackgroundColor;
    QString strColorTotal = colorToString(colorTotal);
    QString strColorHeader = colorToString(colorHeader);

    ui->lblColorHeader->setStyleSheet("QLabel {background-color: "+strColorHeader+"}");
    ui->lblColorTotal->setStyleSheet("QLabel {background-color: "+strColorTotal+"}");

    connect(ui->spnColCount, QOverload<int>::of(&QSpinBox::valueChanged), [=](int i) {
        ui->colTable->setRowCount(i);
    });

    if (this->exec()) {
        m_crossTab->setRowHeight(ui->spnRowHeight->value());
        m_crossTab->setTotalByRowVisible(ui->chkRowTotal->isChecked());
        m_crossTab->setTotalByColumnVisible(ui->chkColTotal->isChecked());
        m_crossTab->setSubTotalVisible(ui->chkColSubTotal->isChecked());
        m_crossTab->setHeaderVisible(ui->chckHeader->isChecked());
        m_crossTab->setColCount(ui->spnColCount->value());

        int startT = ui->lblColorTotal->styleSheet().indexOf("rgba(",0,Qt::CaseInsensitive);
        int endT = ui->lblColorTotal->styleSheet().indexOf(")",Qt::CaseInsensitive)+1;
        int startH = ui->lblColorHeader->styleSheet().indexOf("rgba(",0,Qt::CaseInsensitive);
        int endH = ui->lblColorHeader->styleSheet().indexOf(")",Qt::CaseInsensitive)+1;

        QString strColorT = ui->lblColorTotal->styleSheet().mid(startT,endT-startT);
        QString strColorH = ui->lblColorHeader->styleSheet().mid(startH,endH-startH);

        m_crossTab->totalBackgroundColor = colorFromString(strColorT);
        m_crossTab->headerBackgroundColor = colorFromString(strColorH);

        m_crossTab->columns.clear();
        for (int i = 0; i < ui->colTable->rowCount(); i++) {
            RptCrossTabObject::ColumnParameters column;
            if (ui->colTable->item(i,0) != nullptr)
                column.caption = ui->colTable->item(i,0)->text();
            if (ui->colTable->item(i,1) != nullptr)
                column.value   = ui->colTable->item(i,1)->text();
            if (ui->colTable->item(i,2) != nullptr)
                column.width   = ui->colTable->item(i,2)->text().toInt();

            m_crossTab->columns << column;
        }

        return QDialog::Accepted;
    } else {
        return QDialog::Rejected;
    }
}

void EditFldDlg::update_preview()
{
    if (sender() == ui->bstyle)
        ui->wBarcode->setBarcodeType((BarCode::BarcodeTypes)ui->wBarcode->metaObject()->enumerator(0).value(ui->bstyle->currentIndex()));
    if (sender() == ui->cbFrameType)
        ui->wBarcode->setFrameType((BarCode::FrameTypes)ui->wBarcode->metaObject()->enumerator(1).value(ui->cbFrameType->currentIndex()));
    if (sender() == ui->spnHeight)
        ui->wBarcode->setHeight(ui->spnHeight->value());
}

void EditFldDlg::setScaledContents(bool value)
{
    ui->label->setScaledContents(value);
}

void EditFldDlg::chartTypeChanged(int index)
{
    Q_UNUSED(index);

    if (index != -1)
        m_chart->removeAllSeries();

    ui->lblHoleSize->setVisible(false);
    ui->spnHoleSize->setVisible(false);

    if (ui->cbChartType->currentData().toInt() == QAbstractSeries::SeriesTypeLine) {
        ui->lblChartTypePic->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/lineChart.png")));

        if (index != -1) {
            auto series = new QLineSeries();
            series->append(0,0);
            series->append(1,3);
            series->setName(QString("Series #%1").arg(m_chart->series().size()));

            m_chart->addSeries(series);
            m_chart->createDefaultAxes();
        }
    }
    else if (ui->cbChartType->currentData().toInt() == QAbstractSeries::SeriesTypeBar) {
        ui->lblChartTypePic->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/barChart.png")));

        if (index != -1) {
            auto set0 = new QBarSet("Jane");
            auto set1 = new QBarSet("John");
            auto set2 = new QBarSet("Axel");
            auto set3 = new QBarSet("Mary");
            auto set4 = new QBarSet("Sam");

            *set0 << 1 << 2 << 3 << 4 << 5 << 6;
            *set1 << 5 << 0 << 0 << 4 << 0 << 7;
            *set2 << 3 << 5 << 8 << 13 << 8 << 5;
            *set3 << 5 << 6 << 7 << 3 << 4 << 5;
            *set4 << 9 << 7 << 5 << 3 << 1 << 2;

            auto series = new QBarSeries();
            series->append(set0);
            series->append(set1);
            series->append(set2);
            series->append(set3);
            series->append(set4);

            m_chart->addSeries(series);
        }
    }
    else if (ui->cbChartType->currentData().toInt() == QAbstractSeries::SeriesTypeStackedBar) {
        ui->lblChartTypePic->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/barStackedChart.png")));

        if (index != -1) {
            auto set0 = new QBarSet("Jane");
            auto set1 = new QBarSet("John");
            auto set2 = new QBarSet("Axel");
            auto set3 = new QBarSet("Mary");
            auto set4 = new QBarSet("Sam");

            *set0 << 1 << 2 << 3 << 4 << 5 << 6;
            *set1 << 5 << 0 << 0 << 4 << 0 << 7;
            *set2 << 3 << 5 << 8 << 13 << 8 << 5;
            *set3 << 5 << 6 << 7 << 3 << 4 << 5;
            *set4 << 9 << 7 << 5 << 3 << 1 << 2;

            auto series = new QStackedBarSeries();
            series->append(set0);
            series->append(set1);
            series->append(set2);
            series->append(set3);
            series->append(set4);

            m_chart->addSeries(series);
            m_chart->setTitle("Simple stackedbarchart example");

            QStringList categories;
            categories << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun";
            auto axis = new QBarCategoryAxis();
            axis->append(categories);
            m_chart->addAxis(axis, Qt::AlignBottom);
            series->attachAxis(axis);
            m_chart->legend()->setVisible(true);
        }
    }
    else if (ui->cbChartType->currentData().toInt() == QAbstractSeries::SeriesTypePie) {
        ui->lblChartTypePic->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/pieChart.png")));
        ui->lblHoleSize->setVisible(true);
        ui->spnHoleSize->setVisible(true);

        if (index != -1) {
            auto series = new QPieSeries();
            series->append("Jane", 1);
            series->append("Joe", 2);
            series->append("Andy", 3);
            series->append("Barbara", 4);
            series->append("Axel", 5);

            auto slice = series->slices().at(1);
            slice->setExploded();
            slice->setLabelVisible();
            slice->setPen(QPen(Qt::darkGreen, 2));
            slice->setBrush(Qt::green);

            m_chart->addSeries(series);
        }
    }


    fillSeriesTbl();
}

void EditFldDlg::addSeries()
{
    QAbstractSeries *abstrSeries;
    if (ui->cbChartType->currentData() == QAbstractSeries::SeriesTypeLine) {
        QLineSeries *series = new QLineSeries();
        series->append(0,0);
        series->append(1,3);
        series->setName(QString("Series #%1").arg(m_chart->series().size()));
        abstrSeries = series;

        m_chart->addSeries(series);
        m_chart->createDefaultAxes();

    } else if (ui->cbChartType->currentData() == QAbstractSeries::SeriesTypeBar) {
        auto set = new QBarSet("New set");
        *set << 1 << 2 << 3 << 4 << 5 << 6;

        abstrSeries = m_chart->series().at(0);
        auto series = qobject_cast<QBarSeries*>(abstrSeries);
        series->append(set);
    } else if (ui->cbChartType->currentData() == QAbstractSeries::SeriesTypeStackedBar) {
        auto set = new QBarSet("New set");
        *set << 1 << 2 << 3 << 4 << 5 << 6;

        abstrSeries = m_chart->series().at(0);
        auto series = qobject_cast<QStackedBarSeries*>(abstrSeries);
        series->append(set);
    } else if (ui->cbChartType->currentData() == QAbstractSeries::SeriesTypePie) {
        abstrSeries = m_chart->series().at(0);
        auto series = qobject_cast<QPieSeries*>(abstrSeries);
        series->append("New pie", 1);
    }

    fillSeriesTbl();
}

void EditFldDlg::seriesValue()
{
    if (sender() == ui->btnAddValue) {
        ui->tblSeriesValues->setRowCount(ui->tblSeriesValues->rowCount()+1);
    }
    if (sender() == ui->btnRemoveValue) {
        auto row = ui->tblSeriesValues->currentRow();
        ui->tblSeriesValues->removeRow(row);
    }
}

void EditFldDlg::removeSeries()
{
    QAbstractSeries *abstrSeries;
    auto row = ui->tableSeries->currentRow();
    if (ui->cbChartType->currentData() == QAbstractSeries::SeriesTypeLine) {
        auto series = m_chart->series().at(row);
        m_chart->removeSeries(series);
    } else if (ui->cbChartType->currentData() == QAbstractSeries::SeriesTypeBar) {
        abstrSeries = m_chart->series().at(0);
        auto series = qobject_cast<QBarSeries*>(abstrSeries);
        auto set = series->barSets().at(row);
        series->remove(set);
    } else if (ui->cbChartType->currentData() == QAbstractSeries::SeriesTypeStackedBar) {
        abstrSeries = m_chart->series().at(0);
        auto series = qobject_cast<QStackedBarSeries*>(abstrSeries);
        auto set = series->barSets().at(row);
        series->remove(set);
    } else if (ui->cbChartType->currentData() == QAbstractSeries::SeriesTypePie) {
        abstrSeries = m_chart->series().at(0);
        auto series = qobject_cast<QPieSeries*>(abstrSeries);
        auto slice = series->slices().at(row);
        series->remove(slice);
    }

    fillSeriesTbl();
}

void EditFldDlg::fillSeriesTbl()
{
    ui->tableSeries->setColumnCount(2);

    if (ui->cbChartType->currentData() == QAbstractSeries::SeriesTypePie) {
        ui->tableSeries->horizontalHeaderItem(0)->setText("Name");

        auto series = qobject_cast<QPieSeries*>(m_chart->series().at(0));

        ui->tableSeries->setRowCount(series->count());
        for (int row = 0; row < series->count(); row++) {
            auto slice = series->slices().at(row);
            auto name = slice->label();
            auto newItem = new QTableWidgetItem(name);
            ui->tableSeries->setItem(row, 0, newItem);

            //-- color box and button for selecting color
            auto sc = new SelectColor(ui->tableSeries, colorToString(slice->color()));
            sc->button->setVisible(false);
            //QObject::connect(sc->button, SIGNAL(clicked()), this, SLOT(selectGraphColor()));
            ui->tableSeries->setCellWidget(row, 1, sc);
        }
    }

    if (ui->cbChartType->currentData() == QAbstractSeries::SeriesTypeLine) {
        ui->tableSeries->horizontalHeaderItem(0)->setText("Series name");
        ui->tableSeries->setRowCount(m_chart->series().size());


        for (int row = 0; row < m_chart->series().size(); row++) {
            auto series = qobject_cast<QLineSeries*>(m_chart->series().at(row));
            auto name = series->name();
            auto newItem = new QTableWidgetItem(name);
            ui->tableSeries->setItem(row, 0, newItem);

            //-- color box and button for selecting color
            auto sc = new SelectColor(ui->tableSeries, colorToString(series->color()));
            sc->button->setVisible(false);
            //QObject::connect(sc->button, SIGNAL(clicked()), this, SLOT(selectGraphColor()));
            ui->tableSeries->setCellWidget(row, 1, sc);
        }
    }

    if (ui->cbChartType->currentData() == QAbstractSeries::SeriesTypeStackedBar ||
            ui->cbChartType->currentData() == QAbstractSeries::SeriesTypeBar) {
        ui->tableSeries->horizontalHeaderItem(0)->setText("Bar set name");

        ui->tableSeries->setRowCount(0);
        if (m_chart->series().size() == 0) return;

        QList<QBarSet*> barSetList;
        if (ui->cbChartType->currentData() == QAbstractSeries::SeriesTypeBar) {
            auto series = qobject_cast<QBarSeries*>(m_chart->series().at(0));
            barSetList = series->barSets();
        }

        if (ui->cbChartType->currentData() == QAbstractSeries::SeriesTypeStackedBar) {
            auto series = qobject_cast<QStackedBarSeries*>(m_chart->series().at(0));
            barSetList = series->barSets();
        }


        ui->tableSeries->setRowCount(barSetList.size());
        for (int row = 0; row < barSetList.size(); row++) {
            auto barSet = barSetList.at(row);
            auto name = barSet->label();
            auto newItem = new QTableWidgetItem(name);
            ui->tableSeries->setItem(row, 0, newItem);

            //-- color box and button for selecting color
            auto sc = new SelectColor(ui->tableSeries, colorToString(barSet->color()));
            sc->button->setVisible(false);
            //QObject::connect(sc->button, SIGNAL(clicked()), this, SLOT(selectGraphColor()));
            ui->tableSeries->setCellWidget(row, 1, sc);
        }
    }
}

int EditFldDlg::showDiagram(QGraphicsItem *gItem)
{
    this->setWindowTitle("Chart's property");

    ui->cbChartType->clear();
    ui->cbChartType->addItem("Line chart", QAbstractSeries::SeriesTypeLine);
    ui->cbChartType->addItem("Bar chart", QAbstractSeries::SeriesTypeBar);
    ui->cbChartType->addItem("Bar stacked chart", QAbstractSeries::SeriesTypeStackedBar);
    ui->cbChartType->addItem("Pie chart", QAbstractSeries::SeriesTypePie);

    auto cont = qgraphicsitem_cast<GraphicsBox*>(gItem);
    m_chart = cont->getChart();


    ui->stackedWidget->setCurrentIndex(2);

    if (m_chart->series().size() > 0 && m_chart->series().at(0)->type() == QAbstractSeries::SeriesTypePie) {
        auto series = qobject_cast<QPieSeries*>(m_chart->series().at(0));
        ui->spnHoleSize->setValue(series->holeSize());
    }

    ui->chkStaticChart->setChecked(m_chart->property("staticChart").toInt());
    ui->chkShowLegend->setChecked(m_chart->legend()->isVisible());
    ui->edtCaption->setText(m_chart->title());
    ui->fntTitle->setCurrentFont(m_chart->titleFont());
    ui->fntLegend->setCurrentFont(m_chart->legend()->font());

    QString strColor = colorToString(m_chart->titleBrush().color());
    ui->lblColorTitle->setStyleSheet("QLabel {background-color: "+strColor+"}");

    strColor = colorToString(m_chart->backgroundBrush().color());
    ui->lblColorBackground->setStyleSheet("QLabel {background-color: "+strColor+"}");

    strColor = colorToString(m_chart->legend()->labelBrush().color());
    ui->lblColorLegend->setStyleSheet("QLabel {background-color: "+strColor+"}");

    int currentIndex;
    if (m_chart->legend()->alignment() == Qt::AlignTop) currentIndex = 0;
    if (m_chart->legend()->alignment() == Qt::AlignBottom) currentIndex = 1;
    if (m_chart->legend()->alignment() == Qt::AlignLeft) currentIndex = 2;
    if (m_chart->legend()->alignment() == Qt::AlignRight) currentIndex = 3;

    ui->cbLegendAligment->setCurrentIndex(currentIndex);

    if (m_chart->series().size() > 0) {
        int index = ui->cbChartType->findData(m_chart->series().at(0)->type());
        ui->cbChartType->setCurrentIndex(index);
        chartTypeChanged(-1);
    }

    fillSeriesTbl();

    QObject::connect(ui->cbChartType, SIGNAL(currentIndexChanged(int)), this, SLOT(chartTypeChanged(int)));


    if (this->exec()) {
        QObject::disconnect(ui->cbChartType, SIGNAL(currentIndexChanged(int)), this, SLOT(chartTypeChanged(int)));

        Qt::Alignment alig;
        if (ui->cbLegendAligment->currentIndex() == 0) alig = Qt::AlignTop;
        if (ui->cbLegendAligment->currentIndex() == 1) alig = Qt::AlignBottom;
        if (ui->cbLegendAligment->currentIndex() == 2) alig = Qt::AlignLeft;
        if (ui->cbLegendAligment->currentIndex() == 3) alig = Qt::AlignRight;

        m_chart->legend()->setAlignment(alig);
        m_chart->legend()->setVisible(ui->chkShowLegend->isChecked());
        m_chart->setTitle(ui->edtCaption->text());
        m_chart->setProperty("staticChart", ui->chkStaticChart->isChecked());
        m_chart->setTitleFont(ui->fntTitle->currentFont());
        m_chart->legend()->setFont(ui->fntLegend->currentFont());

        QColor color = colorFromString(ui->lblColorTitle->styleSheet());
        QBrush brush = m_chart->titleBrush();
        brush.setColor(color);
        m_chart->setTitleBrush(brush);

        color = colorFromString(ui->lblColorBackground->styleSheet());
        brush = m_chart->backgroundBrush();
        brush.setColor(color);
        brush.setStyle(Qt::SolidPattern);
        m_chart->setBackgroundBrush(brush);

//        QLinearGradient backgroundGradient;
//        backgroundGradient.setStart(QPointF(0, 0));
//        backgroundGradient.setFinalStop(QPointF(0, 1));
//        backgroundGradient.setColorAt(0.0, QRgb(0xd2d0d1));
//        backgroundGradient.setColorAt(1.0, color/*QRgb(0x4c4547)*/);
//        backgroundGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
//        m_chart->setBackgroundBrush(backgroundGradient);

        color = colorFromString(ui->lblColorLegend->styleSheet());
        brush = m_chart->legend()->labelBrush();
        brush.setColor(color);
        m_chart->legend()->setLabelBrush(brush);


        if (m_chart->series().at(0)->type() == QAbstractSeries::SeriesTypePie) {
            auto series = qobject_cast<QPieSeries*>(m_chart->series().at(0));
            series->setHoleSize(ui->spnHoleSize->value());
        }

        return QDialog::Accepted;
    } else {
        QObject::disconnect(ui->cbChartType, SIGNAL(currentIndexChanged(int)), this, SLOT(chartTypeChanged(int)));
        return QDialog::Rejected;
    }
}

void EditFldDlg::loadImage()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
        QPixmap p;
        p.load(fileName);        
        ui->label->setPixmap(p);
        ui->label->setScaledContents(ui->chkIgnoreAspectRatio->isChecked());
        QFileInfo fi(fileName);
        m_imgFormat = fi.suffix().toUpper();
    }
}

void EditFldDlg::saveImage()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image As"),
                                                    QCoreApplication::applicationDirPath(),
                                                    tr("Images (*.png)"));
    if (!fileName.isEmpty()) {
        #if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
            QPixmap p = QPixmap(ui->label->pixmap(Qt::ReturnByValue));
        #else
            QPixmap p = QPixmap(*ui->label->pixmap());
        #endif

        if (p.isNull())
            return;
        p.save(fileName, m_imgFormat.toLatin1().data());
    }
}

void EditFldDlg::autoFillData(bool value)
{
    ui->tabDiagram->setTabEnabled(1,value);
}

EditFldDlg::~EditFldDlg()
{
    delete ui;
}
