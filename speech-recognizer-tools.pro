#-------------------------------------------------
#
# Project created by QtCreator 2013-03-18T01:23:52
#
#-------------------------------------------------

QT += core gui
QT += multimedia
CONFIG += qwt
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = speech-recognizer-tools
TEMPLATE = app

LIBS += /home/vzhitko/SRC/aquila/aquila/lib/libAquila.a
LIBS += /home/vzhitko/SRC/aquila/aquila/lib/libOoura_fft.a

INCLUDEPATH += /home/vzhitko/SRC/aquila/aquila/include

SOURCES += main.cpp\
        mainwindow.cpp \
    speechproc.cpp \
    addtext.cpp \
    worditem.cpp \
    recorder.cpp \
    settings.cpp \
    player.cpp \
    plotter.cpp \
    SpeechDetector.cpp \
    spectrumplotter.cpp

HEADERS  += mainwindow.h \
    consts.h  \
    speechproc.h\
    addtext.h \
    worditem.h \
    recorder.h \
    settings.h \
    player.h \
    plotter.h \
    SpeechDetector.h \
    spectrumplotter.h

FORMS    += mainwindow.ui \
    addtext.ui \
    settings.ui \
    player.ui

RESOURCES += \
    icons.qrc
