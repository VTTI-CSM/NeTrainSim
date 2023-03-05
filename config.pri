TARGET = NeTrainSim
VERSION = 0.0.1


DEFINES += AS_CMD       #Un-remark this line, if you want to build NeTrainSim as GUI

DEFINES += MYAPP_TARGET=\\\"$${TARGET}\\\" \
           MYAPP_VERSION=\\\"$${VERSION}\\\"
