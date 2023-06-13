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

#include <QWidget>
#include <QDomDocument>

namespace Ui {
class ScriptEditor;
}

class ScriptEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ScriptEditor(QSharedPointer<QDomDocument> xmlDoc, QWidget *parent = nullptr);
    QDomElement saveParamToXML(QSharedPointer<QDomDocument> xmlDoc, QDomElement element);
    void showScript();
    void clear();
    ~ScriptEditor();

private:
    Ui::ScriptEditor *ui;
    QSharedPointer<QDomDocument> m_xmlDoc;

};

