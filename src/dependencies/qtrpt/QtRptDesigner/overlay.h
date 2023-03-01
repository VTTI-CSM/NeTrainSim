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

#ifndef OVERLAY_H
#define OVERLAY_H

#include <QWidget>
#include <QPainter>
#include <QPoint>
#include "RptContainer.h"

typedef QList<RptContainer *> ContList;

class Overlay : public QWidget
{
    Q_OBJECT
public:
    Overlay(QWidget *parent);    
    QWidget *ww;

private:
    ContList allContList;

protected:
    void paintEvent(QPaintEvent *event);

};

#endif // OVERLAY_H
