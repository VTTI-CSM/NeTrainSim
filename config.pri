DEFINES += AS_CMD       #Un-remark this line, if you want to build NeTrainSim as GUI

EXECUTABLE_FILENAME = NeTrainSim
VERSION = 0.0.9



# Define simulator variables
contains(DEFINES, AS_CMD) {
    TARGET = $${EXECUTABLE_FILENAME}
}
else {
    TARGET = $${EXECUTABLE_FILENAME}GUI
}


DEFINES += MYAPP_TARGET=\\\"$${TARGET}\\\" \
           MYAPP_VERSION=\\\"$${VERSION}\\\"
