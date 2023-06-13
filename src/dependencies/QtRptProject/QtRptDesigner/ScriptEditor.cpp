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

#include "ScriptEditor.h"
#include "ui_ScriptEditor.h"
#include "SQLHighlighter.h"
#include <QDebug>
#include <QAction>

ScriptEditor::ScriptEditor(QSharedPointer<QDomDocument> xmlDoc, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScriptEditor)
{
    ui->setupUi(this);
    m_xmlDoc = xmlDoc;

    QSettings settings(QCoreApplication::applicationDirPath()+"/setting.ini",QSettings::IniFormat);
    #if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        settings.setIniCodec("UTF-8");
    #endif

    new SQLHighlighter(ui->editor->document(), &settings);

    QObject::connect(ui->editor, &QPlainTextEdit::textChanged, [=] {
        auto act2 = this->parentWidget()->parentWidget()->parentWidget()->parentWidget()->findChild<QAction *>("actSaveReport");
        act2->setEnabled(true);
    });
    QObject::connect(ui->btnClose, &QPushButton::clicked, [=] {
        auto act2 = this->parentWidget()->parentWidget()->parentWidget()->parentWidget()->findChild<QAction *>("actScriptEditing");
        act2->setChecked(false);
        emit act2->triggered();
    });

    ui->editor->blockSignals(true);
}

void ScriptEditor::showScript()
{
    clear();

    ui->editor->blockSignals(true);

    QDomElement documentElement = m_xmlDoc->documentElement();
    QDomNodeList elements = documentElement.elementsByTagName("Script");

    if (elements.size() > 0) {
        QDomNode cdataNode = elements.at(0).childNodes().at(0);
        if (cdataNode.isCDATASection()) {
            QString script = cdataNode.toCDATASection().data();
            ui->editor->setPlainText(script);
        }
    }

    ui->editor->blockSignals(false);
}

QDomElement ScriptEditor::saveParamToXML(QSharedPointer<QDomDocument> xmlDoc, QDomElement element)
{
    auto elem = xmlDoc->createElement("Script");
    element.appendChild(elem);

    auto data = xmlDoc->createCDATASection(ui->editor->toPlainText());
    elem.appendChild(data);

    return elem;
}

void ScriptEditor::clear()
{
    ui->editor->blockSignals(true);
    ui->editor->clear();
    ui->editor->blockSignals(false);
}

ScriptEditor::~ScriptEditor()
{
    delete ui;
}
