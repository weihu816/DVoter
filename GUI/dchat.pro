#-------------------------------------------------
#
# This is your project .pro file
#
#-------------------------------------------------

QT += core gui
QT += concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Some variables that help the compiler do its thing:
# DEPENDPATH += .
# INCLUDEPATH += .

TARGET = dchat
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
    DNet.cpp \
    DNode.cpp \
    Handler.cpp \
    Member.cpp \
    Queue.cpp \
    MainWindow.cpp

HEADERS  += \
    DNet.h \
    DNode.h \
    Handler.h \
    Member.h \
    Queue.h \
    stdincludes.h \
    MainWindow.h

FORMS += \
    MainWindow.ui
