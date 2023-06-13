/*
Name: XYZ
Version: 2.0.0
Web-site: http://www.qtrpt.tk
Programmer: Aleksey Osipov
E-mail: aliks-os@ukr.net
Web-site: http://www.aliks-os.tk

Copyright 2012-2016 Aleksey Osipov

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

#include "XYZ_Container.h"
#include <QApplication>
#include <QPainter>
#include <QMouseEvent>

XYZContainer::XYZContainer(QWidget *parent, QPoint p, QWidget *cWidget) : QWidget(parent) {
    mode = NONE;
    menu = 0;
    childWidget = cWidget;
    setAttribute(Qt::WA_DeleteOnClose);
    this->setAutoFillBackground(false);
    this->setMouseTracking(true);
    this->setFocusPolicy(Qt::ClickFocus);
    this->setFocus();
    this->move(p);
    this->setVisible(true);

    vLayout = new QVBoxLayout(this);
    if (cWidget != 0) {
        cWidget->setParent(this);
        cWidget->releaseMouse();
        cWidget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        cWidget->installEventFilter(this);   //////////////////
        vLayout->addWidget(cWidget);
        vLayout->setContentsMargins(0,0,0,0);
    }
    this->setLayout(vLayout);

    m_showMenu = false;
    m_isDesigning = true;
    m_isEditing = false;
    m_selected = false;
    m_allowResize = true;
    m_allowDrawSelection = true;
    m_hasOverlay = false;
    scale = 1;
    this->installEventFilter(parent);
}

void XYZContainer::setDesignMode(bool mode) {
    m_isDesigning = mode;
    setEditMode(!mode);
}

void XYZContainer::setEditMode(bool mode) {
    m_isEditing = mode;
    if (mode) {
        childWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
        childWidget->setFocusProxy(0);
        childWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        setCursor(QCursor(Qt::ArrowCursor));
    } else {
        childWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
        childWidget->setFocusProxy(this);
        childWidget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    }
}

bool XYZContainer::isDesigning() {
    return m_isDesigning;
}

bool XYZContainer::isEditing() {
    return m_isEditing;
}

void XYZContainer::setScale(qreal scale) {
    this->scale = scale;
    resize(this->geometry().width()*scale,this->geometry().height()*scale);
}

XYZContainer::~XYZContainer() {
    delete vLayout;
}

void XYZContainer::setChildWidget(QWidget *cWidget) {
    if (cWidget != 0) {
        childWidget = cWidget;
        childWidget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        childWidget->setParent(this);
        vLayout->addWidget(cWidget);
        vLayout->setContentsMargins(0,0,0,0);
    }
}

void XYZContainer::popupShow(const QPoint &pt) {
    if (menu == 0) return;
    if (menu->isEmpty()) return;
    QPoint global = this->mapToGlobal(pt);
    m_showMenu = true;
    menu->exec(global);
    m_showMenu = false;
}

bool XYZContainer::isSelected() {
    return m_selected;
}

void XYZContainer::setSelected(bool value, bool repaint) {
    m_selected = value;
    if (value) {
        this->parentWidget()->installEventFilter(this);
    } else {
        mode = NONE;
    }
    if (repaint)
        this->parentWidget()->repaint();
}

void XYZContainer::setPasted(bool value) {
    m_pasting = value;
}

void XYZContainer::focusInEvent(QFocusEvent *e) {
    Q_UNUSED(e);
    emit inFocus(true);
}

void XYZContainer::focusOutEvent(QFocusEvent *e) {
    Q_UNUSED(e);
    if (QApplication::keyboardModifiers() == Qt::ShiftModifier) return;
    if (!m_isDesigning) return;
    if (m_showMenu) return;
    mode = NONE;
    emit inFocus(false);
    m_selected = false;
    m_pasting = false;
    if (this->parentWidget() != 0 && !m_hasOverlay)
        this->parentWidget()->repaint();
}

bool XYZContainer::eventFilter( QObject *obj, QEvent *evt ) {
    if (m_isDesigning && m_selected && m_allowDrawSelection) {
        QWidget *w = this->parentWidget();
        if (w == obj && evt->type()==QEvent::Paint && m_isDesigning && !m_hasOverlay) {
            //Draw container selection
            QPainter painter(w);
            QPoint p = this->mapTo(w,QPoint(-3,-3));
            QPoint LT = w->mapFrom(w,p);
            QPoint LB = w->mapFrom(w,QPoint(p.x(),p.y()+this->height()));
            QPoint RB = w->mapFrom(w,QPoint(p.x()+this->width(),p.y()+this->height()));
            QPoint CB = w->mapFrom(w,QPoint(p.x()+this->width()/2,p.y()+this->height()));
            QPoint RT = w->mapFrom(w,QPoint(p.x()+this->width(),p.y()));
            QPoint CT = w->mapFrom(w,QPoint(p.x()+this->width()/2,p.y()));

            painter.fillRect(LT.x(),LT.y(),6,6,QColor("black"));
            painter.fillRect(LB.x(),LB.y(),6,6,QColor("black"));
            painter.fillRect(RB.x(),RB.y(),6,6,QColor("black"));
            painter.fillRect(CB.x(),CB.y(),6,6,QColor("black"));
            painter.fillRect(RT.x(),RT.y(),6,6,QColor("black"));
            painter.fillRect(CT.x(),CT.y(),6,6,QColor("black"));
            return QWidget::eventFilter(obj,evt);
        }
    }
    if (m_isDesigning && obj == childWidget && evt->type() == QEvent::FocusIn) {
        this->setFocus();
    }
    return QWidget::eventFilter(obj, evt);
}

void XYZContainer::mousePressEvent(QMouseEvent *e) {
    if (!m_isDesigning) return;

    if (QApplication::keyboardModifiers() == Qt::ShiftModifier && !m_pasting) {
        m_selected = !m_selected;
        setSelected(m_selected);
    } else {
        setSelected(true);
    }

    position = QPoint(e->globalX()-geometry().x(), e->globalY()-geometry().y());
    m_oldRect = this->geometry(); //Fix a start geom

    if (!m_selected) return;
    //QWidget::mouseMoveEvent(e);
    if (!e->buttons() & Qt::LeftButton) {
        setCursorShape(e->pos());
        return;
    }
    if(e->button()==Qt::RightButton) {
        popupShow(e->pos());
        e->accept();
    }
}

void XYZContainer::keyPressEvent(QKeyEvent *e) {
    if (!m_isDesigning) return;
    if (e->key() == Qt::Key_Delete) {
        emit deleteByUser();
        this->deleteLater();
        return;
    }    
    const QRect oldRect = this->geometry();
    m_oldRect = oldRect;
    //Move container by key
    if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
        QPoint newPos(this->x(),this->y());
        if (e->key() == Qt::Key_Up) newPos.setY(newPos.y()-1);
        if (e->key() == Qt::Key_Down) newPos.setY(newPos.y()+1);
        if (e->key() == Qt::Key_Left) newPos.setX(newPos.x()-1);
        if (e->key() == Qt::Key_Right) newPos.setX(newPos.x()+1);
        move(newPos);
    }
    //Change size of the container by key
    if (QApplication::keyboardModifiers() == Qt::ShiftModifier) {
        if (e->key() == Qt::Key_Up) resize(width(),height()-1);
        if (e->key() == Qt::Key_Down) resize(width(),height()+1);
        if (e->key() == Qt::Key_Left) resize(width()-1,height());
        if (e->key() == Qt::Key_Right) resize(width()+1,height());
        setBaseSize(width()/scale,height()/scale);
    }
    emit newGeometry(oldRect, this->geometry());
    //m_geomChanged(oldRect, this->geometry());
}

QRect XYZContainer::getOldGeom() {
    return m_oldRect;
}

void XYZContainer::setOldGeom(QRect rect) {
    m_oldRect = rect;
}

void XYZContainer::m_geomChanged(QRect oldRect, QRect newRect) {
    if (m_isDesigning && oldRect != newRect) {
        emit geomChanged(oldRect, newRect);
    }
}

void XYZContainer::resizeEvent(QResizeEvent *e) {
    setBaseSize(width()/scale,height()/scale);
    QWidget::resizeEvent(e);
}

void XYZContainer::setCursorShape(const QPoint &e_pos) {
    const int diff = 3;
    if (!m_allowResize) {
        setCursor(QCursor(Qt::ArrowCursor));
        mode = MOVE;
        return;
    }
    if (
            //Left-Bottom
            ((e_pos.y() > y() + height() - diff) &&          //Bottom
             (e_pos.x() < x()+diff)) ||                      //Left
            //Right-Bottom
            ((e_pos.y() > y() + height() - diff) &&          //Bottom
             (e_pos.x() > x() + width() - diff)) ||          //Right
            //Left-Top
            ((e_pos.y() < y() + diff) &&                     //Top
             (e_pos.x() < x() + diff)) ||                    //Left
            //Right-Top
            ((e_pos.y() < y() + diff) &&                     //Top
             (e_pos.x() > x() + width() - diff))             //Right
       )
    {
        //Left-Bottom
        if ((e_pos.y() > y() + height() - diff) &&           //Bottom
            (e_pos.x() < x() + diff)) {                      //Left
            mode = RESIZEBL;
            setCursor(QCursor(Qt::SizeBDiagCursor));
        }
        //Right-Bottom
        if ((e_pos.y() > y() + height() - diff) &&           //Bottom
            (e_pos.x() > x() + width() - diff)) {            //Right
            mode = RESIZEBR;
            setCursor(QCursor(Qt::SizeFDiagCursor));
        }
        //Left-Top
        if ((e_pos.y() < y() + diff) &&                      //Top
            (e_pos.x() < x() + diff)) {                      //Left
            mode = RESIZETL;
            setCursor(QCursor(Qt::SizeFDiagCursor));
        }
        //Right-Top
        if ((e_pos.y() < y() + diff) &&                      //Top
            (e_pos.x() > x() + width() - diff)) {            //Right
            mode = RESIZETR;
            setCursor(QCursor(Qt::SizeBDiagCursor));
        }
    }
    // проверка положения курсора по горизонтали
    else if ((e_pos.x() < x() + diff) ||             //Left
            ((e_pos.x() > x() + width() - diff))) {  //Right
        if (e_pos.x() < x() + diff) {                //Left
            setCursor(QCursor(Qt::SizeHorCursor));
            mode = RESIZEL;
        } else {                                     //Right
            setCursor(QCursor(Qt::SizeHorCursor));
            mode = RESIZER;
        }
    }
    // проверка положения курсора по вертикали
    else if (((e_pos.y() > y() + height() - diff)) || //Bottom
              (e_pos.y() < y() + diff)) {             //Top
        if (e_pos.y() < y() + diff) {                 //Top
            setCursor(QCursor(Qt::SizeVerCursor));
            mode = RESIZET;
        } else {                                      //Bottom
            setCursor(QCursor(Qt::SizeVerCursor));
            mode = RESIZEB;
        }
    } else {
        setCursor(QCursor(Qt::ArrowCursor));
        mode = MOVE;
    }
}

void XYZContainer::mouseReleaseEvent(QMouseEvent *e) {
    QWidget::mouseReleaseEvent(e);
    if (!m_isDesigning) return;
    m_geomChanged(m_oldRect, this->geometry());
}

void XYZContainer::mouseMoveEvent(QMouseEvent *e) {
    QWidget::mouseMoveEvent(e);
    if (!m_isDesigning) return;
    if (!m_selected) return;
    if (!e->buttons() & Qt::LeftButton) {
        QPoint p = QPoint(e->x()+geometry().x(), e->y()+geometry().y());
        setCursorShape(p);
        return;
    }

    const QRect oldRect = this->geometry();
    if ((mode == MOVE || mode == NONE) && e->buttons() && Qt::LeftButton) {
        QPoint toMove = e->globalPos() - position;
        if (toMove.x() < -10) return;
        if (toMove.x() > this->parentWidget()->width()-this->width()+1) return;
        if (toMove.y() > this->parentWidget()->height()-this->height()+1) return;
        if (toMove.y() < 0) return;

        move(toMove);
        emit newGeometry(oldRect, this->geometry());
        return;
    }
    if ((mode != MOVE && m_allowResize) && e->buttons() && Qt::LeftButton) {
        switch (mode){
            case RESIZETL: {  //Left-Top
                int newwidth = e->globalX() - position.x() - geometry().x();
                int newheight = e->globalY() - position.y() - geometry().y();
                QPoint toMove = e->globalPos() - position;
                resize(this->geometry().width()-newwidth,this->geometry().height()-newheight);
                move(toMove.x(),toMove.y());
                break;
            }
            case RESIZETR: {  //Right-Top
                int newheight = e->globalY() - position.y() - geometry().y();
                QPoint toMove = e->globalPos() - position;
                resize(e->x(),this->geometry().height()-newheight);
                move(this->x(),toMove.y());
                break;
            }
            case RESIZEBL: {  //Left-Bottom
                int newwidth = e->globalX() - position.x() - geometry().x();
                QPoint toMove = e->globalPos() - position;
                resize(this->geometry().width()-newwidth,e->y());
                move(toMove.x(),this->y());
                break;
            }
            case RESIZEB: {   //Bottom
                resize(width(),e->y()); break;}
            case RESIZEL:  {  //Left
                int newwidth = e->globalX() - position.x() - geometry().x();
                QPoint toMove = e->globalPos() - position;
                resize(this->geometry().width()-newwidth,height());
                move(toMove.x(),this->y());
                break;
            }
            case RESIZET:  {  //Top
                int newheight = e->globalY() - position.y() - geometry().y();
                QPoint toMove = e->globalPos() - position;
                resize(width(),this->geometry().height()-newheight);
                move(this->x(),toMove.y());
                break;
            }
            case RESIZER:  {  //Right
                resize(e->x(),height());
                break;
            }
            case RESIZEBR: {  //Right-Bottom
                resize(e->x(),e->y());
                break;
            }
        }
        setBaseSize(width()/scale,height()/scale);
        this->parentWidget()->repaint();
    }

    emit newGeometry(oldRect, this->geometry());
}

void XYZContainer::allowResize(bool value) {
    m_allowResize = value;
}

void XYZContainer::allowDrawSelection(bool value) {
    m_allowDrawSelection = value;
}

bool XYZContainer::isAllowDrawSelection() {
    return m_allowDrawSelection;
}

void XYZContainer::setHasOverlay(bool value) {
    m_hasOverlay = value;
}

bool XYZContainer::hasOverlay() {
    return m_hasOverlay;
}

XYZContainer *XYZContainer::clone() {
    return NULL;
}

QDataStream &operator<<(QDataStream &stream, const XYZContainer &obj) {
    for(int i=0; i<obj.metaObject()->propertyCount(); ++i) {
        if(obj.metaObject()->property(i).isStored(&obj)) {
            stream << obj.metaObject()->property(i).read(&obj);
        }
    }
    return stream;
}

QDataStream &operator>>(QDataStream &stream, XYZContainer &obj) {
    QVariant var;
    for(int i=0; i<obj.metaObject()->propertyCount(); ++i) {
        if(obj.metaObject()->property(i).isStored(&obj)) {
            stream >> var;
            if (!var.isNull())
                obj.metaObject()->property(i).write(&obj, var);
        }
    }
    return stream;
}
