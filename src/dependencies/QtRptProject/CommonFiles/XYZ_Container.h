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

#ifndef XYZ_CONTAINER_H
#define XYZ_CONTAINER_H

#include <QtDebug>
#include <QWidget>
#include <QMenu>
#include <QVBoxLayout>
#include <QMetaProperty>

enum modes{
    NONE = 0,
    MOVE = 1,
    RESIZETL = 2,
    RESIZET = 3,
    RESIZETR = 4,
    RESIZER = 5,
    RESIZEBR = 6,
    RESIZEB = 7,
    RESIZEBL = 8,
    RESIZEL = 9
};

class XYZContainer : public QWidget {
    Q_OBJECT
    Q_PROPERTY(bool Selected READ isSelected WRITE setSelected)
    Q_PROPERTY(bool Overlayed READ hasOverlay WRITE setHasOverlay)

public:
    XYZContainer(QWidget *parent, QPoint p, QWidget *cWidget = 0);
    ~XYZContainer();
    QWidget *childWidget;
    QMenu *menu;
    void setChildWidget(QWidget *cWidget);
    void setSelected(bool value, bool repaint = true);
    void setPasted(bool value);
    bool isSelected();
    bool isDesigning();
    bool isEditing();
    qreal scale;
    void setScale(qreal scale);
    void allowResize(bool value);
    void allowDrawSelection(bool value);
    bool isAllowDrawSelection();
    void setHasOverlay(bool value);
    bool hasOverlay();
    virtual XYZContainer *clone();
    QRect getOldGeom();
    void setOldGeom(QRect rect);
    void emitInFocus(bool mode) {emit inFocus(mode);}

    friend QDataStream &operator<<(QDataStream &stream, const XYZContainer &obj);
    friend QDataStream &operator>>(QDataStream &stream, XYZContainer &obj);

protected:
    int mode;
    QPoint position;
    QVBoxLayout* vLayout;
    void setCursorShape(const QPoint &e_pos);
    bool eventFilter( QObject *obj, QEvent *evt );
    void keyPressEvent(QKeyEvent *);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *e);
    bool m_selected;
    bool m_pasting;
    bool m_showMenu;
    bool m_isDesigning;
    bool m_isEditing;
    void popupShow(const QPoint &pt);

private:    
    QRect m_oldRect;
    bool m_allowResize;
    bool m_allowDrawSelection;
    bool m_hasOverlay;
    void m_geomChanged(QRect oldRect, QRect newRect);

signals:
    void inFocus(bool mode);
    void newGeometry(QRect oldRect, QRect newRect);
    void geomChanged(QRect oldRect, QRect newRect);
    void deleteByUser();

public slots:
    void setDesignMode(bool mode);
    void setEditMode(bool mode);
};

#endif // XYZ_CONTAINER_H
