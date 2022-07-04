#-----------------------------------------------------
#
# Modified from automatically generated configuration
#
#-----------------------------------------------------

QT      -= core gui
# try to make this a test application
# TEMPLATE = lib
#CONFIG  += staticlib
CONFIG  -= c++11

linux {
    contains(QT_ARCH, i386) {
        target.path = ../../bin/linux/g++-x86
    } else:contains(QT_ARCH, x86_64) {
        target.path = ../../bin/linux/g++-x86_64
    }

    INSTALLS       += target
    QMAKE_CXXFLAGS += -DMACRO_PLATFORM_LINUX
}

debug {
    CONFIG -= release
}
release {
    QMAKE_CXXFLAGS += -O3
    TARGET          = common-test-app
}

debug {
    TARGET          = common-test-app-d
    QMAKE_CXXFLAGS += -DMACRO_DEBUG
}
#message($$CONFIG)
contains(CONFIG, release) {
    TARGET = common-test-app
}

QMAKE_CXXFLAGS += -std=c++17
QMAKE_LFLAGS   += -std=c++17
LIBS           += -lsfml-graphics -lsfml-window -lsfml-system \
                  -L/usr/lib/x86_64-linux-gnu

SOURCES += \
    ../src/BitmapFont.cpp              \
    ../src/ConstString.cpp             \
    ../src/CurrentWorkingDirectory.cpp \
    \#../src/TestSuite.cpp               \
    \ # SFML Utilities
    ../src/sf-DrawText.cpp             \
    ../src/sf-DrawRectangle.cpp        \
    ../src/sf-DrawTriangle.cpp         \
    ../src/sf-Util.cpp                 \
    ../src/sf-DrawLine.cpp             \
    ../src/sf-8x8Font.cpp              \
    ../src/sf-8x16Font.cpp

HEADERS += \
    ../inc/common/BitmapFont.hpp              \
    ../inc/common/ConstString.hpp             \
    ../inc/common/Util.hpp                    \
    ../inc/common/CurrentWorkingDirectory.hpp \
    ../inc/common/BitMaskable.hpp             \
    ../inc/common/StorageUnion.hpp            \
    ../inc/common/MultiType.hpp               \
    ../inc/common/TypeList.hpp                \
    ../inc/common/StringUtil.hpp              \
    ../inc/common/TestSuite.hpp               \
    ../inc/common/Grid.hpp                    \
    ../inc/common/ParseOptions.hpp            \
    ../inc/common/SubGrid.hpp                 \
    ../inc/common/Vector2Util.hpp             \
    ../inc/common/Vector2Traits.hpp           \
    ../inc/common/Vector2.hpp                 \
    ../inc/common/SfmlVectorTraits.hpp        \
    ../inc/common/BezierCurves.hpp            \
    \ # SFML Utilities
    ../inc/common/sf/DrawText.hpp             \
    ../inc/common/sf/DrawRectangle.hpp        \
    ../inc/common/sf/DrawTriangle.hpp         \
    ../inc/common/sf/DrawLine.hpp             \
    ../inc/common/sf/Util.hpp                 \
    \ # Private SFML Utility Headers
    ../src/sf-8x8Font.hpp                     \
    ../src/sf-8x16Font.hpp

# unit tests rounded into a single program (note, this is for dev/debug/IDE
# purposes, NOT for general/automated testing)
SOURCES += \
    \#../unit-tests/all-tests-driver.cpp \
    \#../unit-tests/TestGrid.cpp \
    \#../unit-tests/TestUtil.cpp \
    \#../unit-tests/TestMultiType.cpp \
    \#../unit-tests/test-string-utils.cpp
    \#../unit-tests/test-math-utils.cpp
    \#../unit-tests/vector-tests.cpp
    \#../unit-tests/sfutils-tests.cpp
    ../demos/sf-util-demos.cpp
    #../unit-tests/test-ColorString.cpp

INCLUDEPATH += \
    ../inc
