QT      -= core gui
TEMPLATE = app
CONFIG  -= c++11
CONFIG  += c++17
TARGET  = ecs-unit-tests

linux {
    QMAKE_CXXFLAGS += -DMACRO_PLATFORM_LINUX -DMACRO_ARIAJANKE_ECS3_ENABLE_TYPESET_TESTS
    contains(QT_ARCH, i386) {
        LIBS += -L../../bin/linux/g++-x86
    } else:contains(QT_ARCH, x86_64) {
        LIBS += -L../../bin/linux/g++-x86_64 \
                -L/usr/lib/x86_64-linux-gnu
    }
    LIBS += "-L$$PWD/../lib/cul"
}

QMAKE_CXXFLAGS += -std=c++17 -DMACRO_COMPILER_GCC
QMAKE_LFLAGS   += -std=c++17

SOURCES += \
    ../unit-tests/main.cpp \
    ../unit-tests/AvlTreeEntity.cpp \
    ../unit-tests/HashTableEntity.cpp

HEADERS += ../unit-tests/shared.hpp \
    ../inc/ecs-rev3/SharedPtr.hpp

HEADERS += \
    \ # Library Interface
    ../inc/ariajanke/ecs3/AvlTreeEntity.hpp \
    ../inc/ariajanke/ecs3/EntityRef.hpp \
    ../inc/ariajanke/ecs3/HashTableEntity.hpp \
    ../inc/ariajanke/ecs3/defs.hpp \
    ../inc/ariajanke/ecs3/ecs.hpp \
    ../inc/ariajanke/ecs3/entity-common.hpp \
    ../inc/ariajanke/ecs3/Scene.hpp \
    ../inc/ariajanke/ecs3/TypeSet.hpp \
    ../inc/ariajanke/ecs3/FunctionTraits.hpp \
    ../inc/ariajanke/ecs3/SingleSystem.hpp \
    ../inc/ariajanke/ecs3/SharedPtr.hpp \
    \ # Library Private Headers
    ../inc/ariajanke/ecs3/detail/AvlTreeEntity.hpp \
    ../inc/ariajanke/ecs3/detail/HashTableEntity.hpp \
    ../inc/ariajanke/ecs3/detail/defs.hpp \
    ../inc/ariajanke/ecs3/detail/HashMap.hpp \
    ../inc/ariajanke/ecs3/detail/EntityRef.hpp \
    ../inc/ariajanke/ecs3/detail/SingleSystem.hpp
    
INCLUDEPATH += \
    ../lib/cul/inc  \
    ../inc
