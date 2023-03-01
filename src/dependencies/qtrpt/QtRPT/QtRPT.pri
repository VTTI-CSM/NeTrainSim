QT       += gui xml qml sql charts

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets printsupport
    DEFINES += HAVE_QT5
}

include(../CommonFiles/CommonFiles_QtRpt.pri)

INCLUDEPATH += $$PWD

SOURCES += $$PWD/qtrpt.cpp \
    $$PWD/RptDsPlugin.cpp \
           $$PWD/RptSql.cpp \
           $$PWD/RptFieldObject.cpp \
           $$PWD/RptBandObject.cpp \
           $$PWD/RptPageObject.cpp \
           $$PWD/RptCrossTabObject.cpp \
    $$PWD/RptScriptEngine.cpp \
    $$PWD/RptDsInline.cpp \
    $$PWD/RptDsAbstract.cpp
HEADERS += $$PWD/qtrpt.h \
    $$PWD/RptDsPlugin.h \
           $$PWD/qtrptnamespace.h \
           $$PWD/RptSql.h \
           $$PWD/RptFieldObject.h \
           $$PWD/RptBandObject.h \
           $$PWD/RptPageObject.h \
           $$PWD/RptCrossTabObject.h \
    $$PWD/RptScriptEngine.h \
    $$PWD/RptDsInline.h \
    $$PWD/RptDsAbstract.h

RESOURCES += \
    $$PWD/../QtRPT/imagesRpt.qrc
