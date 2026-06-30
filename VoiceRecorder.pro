QT       += core gui multimedia network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = VoiceRecorder
TEMPLATE = app

DEFINES += CSM_TARGET_WIN_GL
QMAKE_PROJECT_DEPTH = 0

include(models/models.pri)
include(Ability/Ability.pri)
include(SignalPage/SignalPage.pri)
include(CustomControl/CustomControl.pri)

SOURCES += \
    main.cpp \
    centralwindow.cpp

HEADERS += \
    centralwindow.h

win32 {
    LIBS += -lsapi -lole32
}

RESOURCES += \
    resources.qrc

DISTFILES +=
