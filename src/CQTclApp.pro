TEMPLATE = lib

TARGET = CQTclApp

QT += widgets

DEPENDPATH += .

QMAKE_CXXFLAGS += -std=c++17

MOC_DIR = .moc

CONFIG += staticlib

SOURCES += \
CQTclApp.cpp \
CQTclCmdArgs.cpp \
CQTclCmdBase.cpp \
CQTclCmdGroup.cpp \
CQTclReadLine.cpp \
\
CQTclUtil.cpp \
CTclUtil.cpp \

HEADERS += \
../include/CQTclApp.h \
../include/CQTclCmdArg.h \
../include/CQTclCmdArgs.h \
../include/CQTclCmdBase.h \
../include/CQTclCmdGroup.h \
../include/CQTclReadLine.h \
\
../include/CQTclUtil.h \
../include/CTclUtil.h \

OBJECTS_DIR = ../obj

DESTDIR = ../lib

INCLUDEPATH += \
. \
../include \
../../CQPerfMonitor/include \
../../CTclUtil/include \
../../CQUtil/include \
../../CReadLine/include \
../../CFile/include \
../../CUtil/include \
../../CStrUtil/include \
../../COS/include \
/usr/include/tcl \
