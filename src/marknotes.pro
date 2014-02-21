#-------------------------------------------------
#
# Project created by QtCreator 2014-02-19T09:59:24
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = marknotes
TEMPLATE = app

LIBS += -lmarkdown

SOURCES += main.cpp\
        mainwindow.cpp \
    discountconverter.cpp

HEADERS  += mainwindow.h \
    markdownconverter.h \
    discountconverter.h

FORMS    += mainwindow.ui

CODECFORTR = UTF-8

TRANSLATIONS = marknotes_en.ts \
               marknotes_cs.ts
