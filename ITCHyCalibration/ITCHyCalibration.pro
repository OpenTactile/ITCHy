#-------------------------------------------------
#
# Project created by QtCreator 2016-01-09T21:45:37
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = ITCHyCalibration
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h
FORMS    += mainwindow.ui

LIBS += -lITCHy -lusb
