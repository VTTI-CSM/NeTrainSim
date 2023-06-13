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

#ifndef XYZ_TEXTEDITOR_H
#define XYZ_TEXTEDITOR_H

#include <QWidget>
#include <QTextEdit>
#include <QTextList>
#include <QCompleter>
#include "XYZ_TextEdit.h"

namespace Ui {
    class XYZTextEditor;
}

class XYZTextEditor : public QWidget {
    Q_OBJECT
public:
    XYZTextEditor(QWidget *parent = nullptr);
    ~XYZTextEditor();
    void setCompleter(QCompleter *c);
    QCompleter *completer() const;
    XYZTextEdit *textEdit;

protected:

private:
    Ui::XYZTextEditor *m_ui;
    QCompleter *c = nullptr;
    void setupTextActions();
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void alignmentChanged(Qt::Alignment a);
    void fontChanged(const QFont &f);

private slots:
    void textSize(const QString &p);
    void textFamily(const QString &f);
    void textStyle(int styleIndex);
    void textBold();
    void textItalic();
    void textUnderline();
    void textAlign();
    void textColor();
    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();
    void textDirection();
};

#endif // XYZ_TEXTEDITOR_H
