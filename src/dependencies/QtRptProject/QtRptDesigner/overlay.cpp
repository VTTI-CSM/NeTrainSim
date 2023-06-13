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

#include "overlay.h"

Overlay::Overlay(QWidget *parent) : QWidget(parent) {
    setPalette(Qt::transparent);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    allContList = ContList();
}

void Overlay::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    allContList = ContList();
    allContList << this->parentWidget()->findChildren<RptContainer *>();

    foreach(RptContainer *cont, allContList) {
        if (cont->isSelected() && cont->isAllowDrawSelection()) {
            if (cont == 0) return;
            if (cont == NULL) return;

            QPoint pc = cont->mapToParent(QPoint(-3,-3));
            QPoint pb = cont->parentWidget()->mapToParent(pc);
            QPoint LT = cont->parentWidget()->parentWidget()->mapToParent(pb);

            QPoint LB = QPoint(LT.x(),LT.y()+cont->height());
            QPoint RB = QPoint(LT.x()+cont->width(),LT.y()+cont->height());
            QPoint CB = QPoint(LT.x()+cont->width()/2,LT.y()+cont->height());
            QPoint RT = QPoint(LT.x()+cont->width(),LT.y());
            QPoint CT = QPoint(LT.x()+cont->width()/2,LT.y());

            painter.fillRect(LT.x(),LT.y(),6,6,QColor("blue"));
            painter.fillRect(LB.x(),LB.y(),6,6,QColor("blue"));
            painter.fillRect(RB.x(),RB.y(),6,6,QColor("blue"));
            painter.fillRect(CB.x(),CB.y(),6,6,QColor("blue"));
            painter.fillRect(RT.x(),RT.y(),6,6,QColor("blue"));
            painter.fillRect(CT.x(),CT.y(),6,6,QColor("blue"));
        }
    }
}
