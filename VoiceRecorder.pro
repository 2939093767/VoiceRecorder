QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = VoiceRecorder
TEMPLATE = app

DEFINES += CSM_TARGET_WIN_GL
QMAKE_PROJECT_DEPTH = 0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    audiorecorder.cpp \
    speechrecognizer.cpp

HEADERS += \
    mainwindow.h \
    audiorecorder.h \
    speechrecognizer.h

FORMS += \
    mainwindow.ui

win32 {
    LIBS += -lsapi -lole32
}

RESOURCES += \
    resources.qrc

DISTFILES +=
