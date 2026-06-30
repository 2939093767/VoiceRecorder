# SignalPage 子页面管理层子项目
# UI子页面管理，承接UI参数、转发业务调用、接收业务返回结果

QT += core gui widgets multimedia

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/pagebase.h \
    $$PWD/voicerecorderpage.h \
    $$PWD/meetingsummarypage.h

SOURCES += \
    $$PWD/pagebase.cpp \
    $$PWD/voicerecorderpage.cpp \
    $$PWD/meetingsummarypage.cpp

FORMS += \
    $$PWD/voicerecorderpage.ui \
    $$PWD/meetingsummarypage.ui
