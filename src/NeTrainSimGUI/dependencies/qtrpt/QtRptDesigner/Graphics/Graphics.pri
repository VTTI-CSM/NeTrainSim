QT       += gui xml

greaterThan(QT_MAJOR_VERSION, 4){
    QT += widgets printsupport
    DEFINES += HAVE_QT5
}

INCLUDEPATH += $$PWD

SOURCES += $$PWD/GraphicsScene.cpp \
    $$PWD/CornerGrabber.cpp \
    $$PWD/GraphicsBox.cpp \
    $$PWD/GraphicsHelperClass.cpp \
    $$PWD/UndoRedoCommands.cpp \
    $$PWD/GraphicsLine.cpp

HEADERS += $$PWD/GraphicsScene.h \
    $$PWD/CornerGrabber.h \
    $$PWD/GraphicsBox.h \
    $$PWD/GraphicsNameSpace.h \
    $$PWD/GraphicsHelperClass.h \
    $$PWD/UndoRedoCommands.h \
    $$PWD/GraphicsLine.h

