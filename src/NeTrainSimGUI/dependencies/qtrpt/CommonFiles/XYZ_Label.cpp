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

#include "XYZ_Label.h"
#include <QDebug>

XYZLabel::XYZLabel(QWidget *parent)
: QLabel(parent)
{
    m_bHover = false;
    setCursor(Qt::PointingHandCursor);
}

XYZLabel::XYZLabel(const QString &Text, QWidget *parent)
: QLabel(Text, parent)
{
    m_bHover = false;
    setCursor(Qt::PointingHandCursor);
}

XYZLabel::~XYZLabel()
{}

void XYZLabel::setHoverText(bool bHover)
{
    m_bHover = bHover;
}

void XYZLabel::enterEvent(QEvent *)
{
    if( m_bHover ) {
        QFont font = this->font();
        font.setUnderline(m_bHover);
        setFont(font);
    }
}

void XYZLabel::leaveEvent(QEvent *)
{
    if( m_bHover ) {
        QFont font = this->font();
        font.setUnderline(false);
        setFont(font);
    }
}

void XYZLabel::mouseReleaseEvent(QMouseEvent *)
{
    if (timer.isActive())
    {
        timer.stop();
        emit doubleClicked();
        qDebug() << "double click";
    }
    else
    {	
        timer.start(300, this);
    }
}

void XYZLabel::timerEvent(QTimerEvent *)
{
    timer.stop();
	emit clicked();
    qDebug() << "single click";
}
