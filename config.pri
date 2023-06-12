#DEFINES += AS_CMD       #Un-remark this line, if you want to build NeTrainSim as GUI

EXECUTABLE_FILENAME = NeTrainSim
GUI_EXECUTABLE_FILENAME = $${EXECUTABLE_FILENAME}GUI
VERSION = 0.1.0



# Define simulator variables
contains(DEFINES, AS_CMD) {
    TARGET = $${EXECUTABLE_FILENAME}
}
else {
    TARGET = $${GUI_EXECUTABLE_FILENAME}
}


DEFINES += MYAPP_TARGET=\\\"$${TARGET}\\\" \
           MYAPP_VERSION=\\\"$${VERSION}\\\"
