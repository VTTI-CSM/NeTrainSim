INCLUDEPATH += $$PWD

SOURCES += $$PWD/diagramobject.cpp \
           $$PWD/diagramdocument.cpp \
           $$PWD/diagramitem.cpp \
           $$PWD/hub.cpp \
           $$PWD/line.cpp \
           $$PWD/connector.cpp \
           $$PWD/databasetable.cpp \
           $$PWD/column.cpp \
           $$PWD/columnlist.cpp \
           $$PWD/databaserelationship.cpp \
           $$PWD/commands.cpp \
           $$PWD/boxsidehub.cpp \
           SQLDiagram/ItemPropertyDlg.cpp
HEADERS += $$PWD/diagramobject.h \
           $$PWD/diagramdocument.h \
           $$PWD/diagramitem.h \
           $$PWD/hub.h \
           $$PWD/line.h \
           $$PWD/connector.h \
           $$PWD/databasetable.h \
           $$PWD/column.h \
           $$PWD/columnlist.h \
           $$PWD/databaserelationship.h \
           $$PWD/commands.h \
           $$PWD/boxsidehub.h \
           SQLDiagram/ItemPropertyDlg.h

FORMS += \
    SQLDiagram/ItemPropertyDlg.ui
