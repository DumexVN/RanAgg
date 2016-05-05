#-------------------------------------------------
#
# Project created by QtCreator 2016-05-04T10:41:18
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = RandomAggreagationLargeGraph
CONFIG   += console
CONFIG   += c++11
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    vertex.cpp \
    edge.cpp \
    graph.cpp

HEADERS += \
    vertex.h \
    edge.h \
    graph.h

INCLUDEPATH += "C:\Boost\boost_1_56_0"
