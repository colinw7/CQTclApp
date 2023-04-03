TEMPLATE = app

TARGET = CQTclAppTest

DEPENDPATH += .

QT += widgets

#CONFIG += debug

# Input
SOURCES += \
CQTclAppShell.cpp \
CQTclAppTest.cpp \
\
CQCommand.cpp \

HEADERS += \
CQTclAppShell.h \
\
CQCommand.h \

DESTDIR     = .
OBJECTS_DIR = .

INCLUDEPATH += \
../include \
../../CQUtil/include \
../../COS/include \
.

unix:LIBS += \
-L../lib \
-L../../CQPerfMonitor/lib \
-L../../CQUtil/lib \
-L../../CReadLine/lib \
-L../../CUtil/lib \
-L../../CFile/lib \
-L../../CRegExp/lib \
-L../../CStrUtil/lib \
-L../../COS/lib \
-lCQTclApp -lCQPerfMonitor -lCQUtil -lCUtil \
-lCReadLine -lCFile -lCRegExp -lCStrUtil -lCOS \
-lreadline -ltcl -ltre
