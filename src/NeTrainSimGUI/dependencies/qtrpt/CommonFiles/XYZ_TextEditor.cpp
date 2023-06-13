/*
Name: XYZ
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

#include <QColorDialog>
#include <QButtonGroup>
#include "XYZ_TextEditor.h"
#include "ui_XYZ_TextEditor.h"
#include <QDebug>
#include <QScrollBar>


XYZTextEditor::XYZTextEditor(QWidget *parent)
: QWidget(parent), m_ui(new Ui::XYZTextEditor)
{
    m_ui->setupUi(this);
    setupTextActions();
    fontChanged(m_ui->textEdit->font());
    alignmentChanged(m_ui->textEdit->alignment());
    m_ui->textEdit->setFocus();
    textEdit = m_ui->textEdit;

    auto btnGroup = new QButtonGroup(this);
    btnGroup->addButton(m_ui->btnAlignCenter);
    btnGroup->addButton(m_ui->btnAlignJustify);
    btnGroup->addButton(m_ui->btnAlignLeft);
    btnGroup->addButton(m_ui->btnAlignRight);
    QObject::connect(m_ui->textEdit, SIGNAL(currentCharFormatChanged(const QTextCharFormat &)),
                     this, SLOT(currentCharFormatChanged(const QTextCharFormat &)));
    QObject::connect(m_ui->textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(cursorPositionChanged()));
    QObject::connect(m_ui->btnCut, SIGNAL(clicked()), m_ui->textEdit, SLOT(cut()));
    QObject::connect(m_ui->btnCopy, SIGNAL(clicked()), m_ui->textEdit, SLOT(copy()));
    QObject::connect(m_ui->btnPaste, SIGNAL(clicked()), m_ui->textEdit, SLOT(paste()));
    QObject::connect(m_ui->btnTextDirection, SIGNAL(clicked()), this, SLOT(textDirection()));

//    m_ui->textEdit->document()->defaultTextOption().setTextDirection(QApplication::layoutDirection());
//    QTextCursor cursor = m_ui->textEdit->textCursor();
//    QTextBlockFormat blockFmt = cursor.blockFormat();
//    if (QApplication::layoutDirection() == Qt::RightToLeft) {
//        blockFmt.setLayoutDirection(Qt::RightToLeft);
//    } else {
//        blockFmt.setLayoutDirection(Qt::LeftToRight);
//    }
//    cursor.setBlockFormat(blockFmt);

//    qDebug() << m_ui->textEdit->document()->defaultTextOption().textDirection();
//    qDebug() << m_ui->textEdit->toHtml();
}

void XYZTextEditor::textDirection()
{
    QTextCursor cursor = m_ui->textEdit->textCursor();
    QTextBlockFormat blockFmt = cursor.blockFormat();

    QTextOption topt = m_ui->textEdit->document()->defaultTextOption();
    if (m_ui->btnTextDirection->isChecked()) {
        topt.setTextDirection(Qt::RightToLeft);
        blockFmt.setLayoutDirection(Qt::RightToLeft);
    } else {
        topt.setTextDirection(Qt::LeftToRight);
        blockFmt.setLayoutDirection(Qt::LeftToRight);
    }
    m_ui->textEdit->document()->setDefaultTextOption(topt);
    cursor.setBlockFormat(blockFmt);
}

XYZTextEditor::~XYZTextEditor()
{
    delete m_ui;
}

void XYZTextEditor::setupTextActions()
{
    QObject::connect(m_ui->btnTextBold, SIGNAL(clicked()), this, SLOT(textBold()));
    QObject::connect(m_ui->btnTextItalic, SIGNAL(clicked()), this, SLOT(textItalic()));
    QObject::connect(m_ui->btnUnderline, SIGNAL(clicked()), this, SLOT(textUnderline()));
    QObject::connect(m_ui->btnAlignCenter, SIGNAL(clicked()), this, SLOT(textAlign()));
    QObject::connect(m_ui->btnAlignJustify, SIGNAL(clicked()), this, SLOT(textAlign()));
    QObject::connect(m_ui->btnAlignLeft, SIGNAL(clicked()), this, SLOT(textAlign()));
    QObject::connect(m_ui->btnAlignRight, SIGNAL(clicked()), this, SLOT(textAlign()));
    QObject::connect(m_ui->btnTextColor, SIGNAL(clicked()), this, SLOT(textColor()));

    m_ui->comboStyle->addItem(tr("Standard"));
    m_ui->comboStyle->addItem(tr("Bullet List (Disc)"));
    m_ui->comboStyle->addItem(tr("Bullet List (Circle)"));
    m_ui->comboStyle->addItem(tr("Bullet List (Square)"));
    m_ui->comboStyle->addItem(tr("Ordered List (Decimal)"));
    m_ui->comboStyle->addItem(tr("Ordered List (Alpha lower)"));
    m_ui->comboStyle->addItem(tr("Ordered List (Alpha upper)"));
    QObject::connect(m_ui->comboStyle, SIGNAL(activated(int)), this, SLOT(textStyle(int)));

    QObject::connect(m_ui->comboFont, SIGNAL(activated(const QString &)), this, SLOT(textFamily(const QString &)));

    m_ui->comboSize->clear();
    QFontDatabase db;
    for (int size : db.standardSizes())
        m_ui->comboSize->addItem(QString::number(size));

    QObject::connect(m_ui->comboSize, SIGNAL(activated(const QString &)), this, SLOT(textSize(const QString &)));
    m_ui->comboSize->setCurrentIndex(m_ui->comboSize->findText(QString::number(QApplication::font().pointSize())));
}

void XYZTextEditor::textAlign()
{
    if (sender()->objectName() == "btnAlignLeft")
        m_ui->textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if (sender()->objectName() == "btnAlignCenter")
        m_ui->textEdit->setAlignment(Qt::AlignHCenter);
    else if (sender()->objectName() == "btnAlignRight")
        m_ui->textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if (sender()->objectName() == "btnAlignJustify")
        m_ui->textEdit->setAlignment(Qt::AlignJustify);
}

void XYZTextEditor::textBold()
{
    QTextCharFormat fmt = m_ui->textEdit->currentCharFormat();
    fmt.setFontWeight(m_ui->btnTextBold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void XYZTextEditor::textUnderline()
{
    QTextCharFormat fmt = m_ui->textEdit->currentCharFormat();
    fmt.setFontUnderline(m_ui->btnUnderline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void XYZTextEditor::textItalic()
{
    QTextCharFormat fmt = m_ui->textEdit->currentCharFormat();
    fmt.setFontItalic(m_ui->btnTextItalic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void XYZTextEditor::textStyle(int styleIndex)
{
    QTextCursor cursor = m_ui->textEdit->textCursor();

    if (styleIndex != 0) {
        QTextListFormat::Style style = QTextListFormat::ListDisc;

        switch (styleIndex) {
            default:
            case 1:
                style = QTextListFormat::ListDisc;
                break;
            case 2:
                style = QTextListFormat::ListCircle;
                break;
            case 3:
                style = QTextListFormat::ListSquare;
                break;
            case 4:
                style = QTextListFormat::ListDecimal;
                break;
            case 5:
                style = QTextListFormat::ListLowerAlpha;
                break;
            case 6:
                style = QTextListFormat::ListUpperAlpha;
                break;
        }

        cursor.beginEditBlock();

        QTextBlockFormat blockFmt = cursor.blockFormat();

        QTextListFormat listFmt;

        if (cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }

        listFmt.setStyle(style);

        cursor.createList(listFmt);

        cursor.endEditBlock();
    } else {
        // ####
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.mergeBlockFormat(bfmt);
    }
}

void XYZTextEditor::textFamily(const QString &f)
{
    QTextCharFormat fmt = m_ui->textEdit->currentCharFormat();
    fmt.setFontFamily(f);
    fmt.setFontPointSize(m_ui->comboSize->currentText().toFloat());
    mergeFormatOnWordOrSelection(fmt);
}

void XYZTextEditor::textSize(const QString &p)
{
    qreal pointSize = p.toFloat();
    if (p.toFloat() > 0)
    {
        QTextCharFormat fmt = m_ui->textEdit->currentCharFormat();
        fmt.setFontFamily(m_ui->comboFont->currentFont().family());
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void XYZTextEditor::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = m_ui->textEdit->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);

    cursor.mergeCharFormat(format);
    m_ui->textEdit->mergeCurrentCharFormat(format);
    m_ui->textEdit->setCurrentCharFormat(format);
}

void XYZTextEditor::alignmentChanged(Qt::Alignment a)
{
    if (a & Qt::AlignLeft) {
        m_ui->btnAlignLeft->setChecked(true);
        m_ui->btnAlignCenter->setChecked(false);
        m_ui->btnAlignRight->setChecked(false);
        m_ui->btnAlignJustify->setChecked(false);
    } else if (a & Qt::AlignHCenter) {
        m_ui->btnAlignLeft->setChecked(false);
        m_ui->btnAlignCenter->setChecked(true);
        m_ui->btnAlignRight->setChecked(false);
        m_ui->btnAlignJustify->setChecked(false);
    } else if (a & Qt::AlignRight) {
        m_ui->btnAlignLeft->setChecked(false);
        m_ui->btnAlignCenter->setChecked(false);
        m_ui->btnAlignRight->setChecked(true);
        m_ui->btnAlignJustify->setChecked(false);
    } else if (a & Qt::AlignJustify) {
        m_ui->btnAlignLeft->setChecked(false);
        m_ui->btnAlignCenter->setChecked(false);
        m_ui->btnAlignRight->setChecked(false);
        m_ui->btnAlignJustify->setChecked(true);
    }
}

void XYZTextEditor::cursorPositionChanged()
{
    alignmentChanged(m_ui->textEdit->alignment());

    QTextBlockFormat blockFmt = m_ui->textEdit->textCursor().blockFormat();
    if (blockFmt.layoutDirection() == Qt::RightToLeft)
        m_ui->btnTextDirection->setChecked(true);
    else
        m_ui->btnTextDirection->setChecked(false);
}

void XYZTextEditor::fontChanged(const QFont &f)
{
    m_ui->comboFont->setCurrentIndex(m_ui->comboFont->findText(QFontInfo(f).family()));
    m_ui->comboSize->setCurrentIndex(m_ui->comboSize->findText(QString::number(f.pointSize())));
    m_ui->btnTextBold->setChecked(f.bold());
    m_ui->btnTextItalic->setChecked(f.italic());
    m_ui->btnUnderline->setChecked(f.underline());
}

void XYZTextEditor::currentCharFormatChanged(const QTextCharFormat &format)
{
    fontChanged(format.font());
}

void XYZTextEditor::textColor()
{
    QColor color;
    QScopedPointer<QColorDialog> dlg(new QColorDialog(this));
    if (dlg->exec() == QDialog::Accepted)
        color = dlg->selectedColor();
    else
        return;

    QTextCharFormat fmt;
    fmt.setForeground( QBrush( color ) );
    mergeFormatOnWordOrSelection(fmt);
}

void XYZTextEditor::setCompleter(QCompleter *completer)
{
    textEdit->setCompleter(completer);
}

QCompleter *XYZTextEditor::completer() const
{
    return textEdit->completer();
}
