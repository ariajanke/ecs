QT      -= core gui
TEMPLATE = app
CONFIG  -= c++11
CONFIG  += c++17

linux {

    QMAKE_CXXFLAGS += -DMACRO_PLATFORM_LINUX -DMACRO_BUILD_WITH_SFML #-DMACRO_ECS_DEADBEEF_SAFETY #-DMACRO_BUILD_WITH_SFML
    contains(QT_ARCH, i386) {
        LIBS += -L../../bin/linux/g++-x86
    } else:contains(QT_ARCH, x86_64) {
        LIBS += -L../../bin/linux/g++-x86_64 \
                -L/usr/lib/x86_64-linux-gnu
    }
    LIBS += "-L$$PWD/../lib/cul"
}

debug {
    TARGET  = testnewecs
}
release {
    TARGET  = testnewecs
}

QMAKE_CXXFLAGS += -std=c++14 -DMACRO_COMPILER_GCC
QMAKE_LFLAGS   += -std=c++14
LIBS           += -lsfml-window -lsfml-system -lsfml-graphics -lcommon-d

SOURCES += ecs_test.cpp
# SOURCES += ecs_test2.cpp

HEADERS += \
    ../inc/ecs/ecs.hpp \
    ../inc/ecs/Entity.hpp \
    ../inc/ecs/EntityRef.hpp \
    ../inc/ecs/ecsdefs.hpp

INCLUDEPATH += \
    ../lib/cul/inc \
    ../inc
