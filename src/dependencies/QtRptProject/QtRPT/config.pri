CONFIG(debug, debug|release) {
    DEST_DIRECTORY = $$PWD/../bin/debug
}
CONFIG(release, debug|release) {
    DEST_DIRECTORY = $$PWD/../bin/release
}


DEFINES += QTRPT_LIBRARY    #Un-remark this line, if you want to build QtRPT as a library
DEFINES += NO_BARCODE       #Un-remark this line, if you want to build QtRPT without BARCODE support
#DEFINES += QXLSX_LIBRARY    #Remark this line, if you want to build QtRPT without XLSX support
