# Ability 业务能力层子项目
# 纯后台业务逻辑，无UI依赖，统一纳入 skill_Ability 命名空间

QT += network multimedia

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/recordmanager.h \
    $$PWD/audiorecorder.h \
    $$PWD/speechrecognizer.h \
    $$PWD/cloudllm.h

SOURCES += \
    $$PWD/recordmanager.cpp \
    $$PWD/audiorecorder.cpp \
    $$PWD/speechrecognizer.cpp \
    $$PWD/cloudllm.cpp

win32 {
    LIBS += -lsapi -lole32
}
