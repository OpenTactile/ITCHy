#-------------------------------------------------
#
# Project created by QtCreator 2016-01-09T22:22:36
#
#-------------------------------------------------

QT       -= core gui
TARGET    = ITCHy
TEMPLATE  = lib
CONFIG   += c++11

INSTALL_PATH_LIB = /usr/lib
INSTALL_PATH_INCLUDE = /usr/include/itchy

DEFINES += LIBITCHY_LIBRARY

SOURCES += \
    itchy.cpp \
    pjrc_rawhid.c

HEADERS += \
    itchy/itchy.h \
    pjrc_rawhid.h

LIBS += -lusb

unix {
    target.path = $${INSTALL_PATH_LIB}
    header_files.path = $${INSTALL_PATH_INCLUDE}
    header_files.files = itchy/itchy.h itchy/itchy
    INSTALLS += target header_files
}
