/*
Name: XYZ
Version: 2.0.1
Programmer: Aleksey Osipov
E-mail: aliks-os@ukr.net
Web-site: http://www.aliks-os.tk

Copyright 2012-2017 Aleksey Osipov

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

#include "XYZ_Logger.h"

/*!
    \class XYZLogger
    \inmodule XYZ_Logger.h
    \brief The XYZLogger class is a object
           helps to log custom messages. Messages
           can save to a file or displays at QPlainTextEdit.
 */

/*!
 \fn XYZLogger::XYZLogger(QObject *parent, QString fileName, QPlainTextEdit *editor) : QObject(parent)
    Constructs a XYZLogger object with the given \a parent.
*/
XYZLogger::XYZLogger(QObject *parent, QString fileName, QPlainTextEdit *editor)
: QObject(parent)
{
    m_editor = editor;
    m_showDate = true;
    if (!fileName.isEmpty()) {
        file = new QFile;
        file->setFileName(fileName);
        file->open(QIODevice::Append | QIODevice::Text);
    }
}

/*!
 \fn XYZLogger::write(const QString &value)
  XYZLogger::write write a \a value to the logger
 */
void XYZLogger::write(const QString &value)
{
    QString text = value;// + "\n";
    if (m_showDate)
        text = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss ") + text;
    QTextStream out(file);
    out.setCodec("UTF-8");
    if (file != nullptr) {
        out << text;
    }
    if (m_editor != nullptr)
        m_editor->appendPlainText(text);
}

/*!
 \fn XYZLogger::setShowDateTime(bool value)
 XYZLogger::setShowDateTime sets \a value show or not date and time during logging
 */
void XYZLogger::setShowDateTime(bool value)
{
    m_showDate = value;
}

//! Destructor
XYZLogger::~XYZLogger()
{
    if (file != nullptr)
        file->close();
}
