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

#ifndef XYZ_LOGGER_H
#define XYZ_LOGGER_H

#include <QObject>
#include <QPlainTextEdit>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

class XYZLogger : public QObject
{
    Q_OBJECT
public:
    explicit XYZLogger(QObject *parent, QString fileName, QPlainTextEdit *editor = 0);
    ~XYZLogger();
    void setShowDateTime(bool value);

private:
    QFile *file;
    QPlainTextEdit *m_editor;
    bool m_showDate;

signals:

public slots:
    void write(const QString &value);

};

#endif // XYZ_LOGGER_H
