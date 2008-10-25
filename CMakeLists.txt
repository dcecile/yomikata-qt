project(yomikata)

# Source files
set(yomikata_SRCS
    debug.cpp

    mainwindow.cpp

    debugwidget.cpp
    debugpageitem.cpp

    

    book.cpp
    revision.cpp
)

set(yomikata_qt_SRCS
    mainqt.cpp
    mainwindowqt.cpp
)

set(yomikata_kde_SRCS
    mainkde.cpp
    mainwindowkde.cpp
)

# Unit test files
set(test_SRCS
    test.cpp
)

# Unit tests
set(test_CLASSES
    book
)

# TODO install into the build directory by default
# TODO add clean_doc and clean_lcov to the clean target

# Minimum CMake version
cmake_minimum_required(VERSION 2.6)

# Options
include(cmake/Options.cmake)

# Dependencies
if(QT_ONLY)
    # Qt-only dependency
    find_package(Qt4 REQUIRED)
    set(QT_USE_QTTEST 1)
    include(${QT_USE_FILE})
    include_directories(${QT4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})
else()
    # KDE4 dependency
    find_package(KDE4 REQUIRED)
    include_directories(${KDE4_INCLUDES})
    include(KDE4Defaults)
endif()

# Source files
if(QT_ONLY)
    set(yomikata_SRCS ${yomikata_SRCS} ${yomikata_qt_SRCS})
    add_definitions(-DQT_ONLY)
else()
    set(yomikata_SRCS ${yomikata_SRCS} ${yomikata_kde_SRCS})
endif()

# Development options
if(DEV_MODE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -O0 -ggdb")
    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install
        CACHE STRING
        "Install path prefix, prepended onto install directories.")
endif()

# Unit test build
if(UNIT_TESTING)
    include(cmake/UnitTesting.cmake)
endif()

# Code coverage build
if(CODE_COVERAGE)
    include(cmake/CodeCoverage.cmake)
endif()

if(QT_ONLY)
    # Qt-only build steps
    qt4_automoc(${yomikata_SRCS})
    add_executable(yomikata ${yomikata_SRCS})
    target_link_libraries(yomikata ${QT_LIBRARIES} ${COVERAGE_LIBRARY})
    set_target_properties(yomikata PROPERTIES COMPILE_FLAGS ${COVERAGE_FLAGS})
else()
    # KDE build steps
    kde4_add_kcfg_files(yomikata_SRCS settings.kcfgc)
    kde4_add_executable(yomikata ${yomikata_SRCS})
    target_link_libraries(yomikata ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${QT_QTTEST_LIBRARY}
        ${COVERAGE_LIBRARY})
    set_target_properties(yomikata PROPERTIES COMPILE_FLAGS ${COVERAGE_FLAGS})
endif()

# Doxygen
include(cmake/Doxygen.cmake)

if(QT_ONLY)
    # Qt-only install
else()
    # KDE install
    install(TARGETS yomikata DESTINATION ${BIN_INSTALL_DIR})
    install(FILES yomikata.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR})
    install(FILES yomikata.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
    install(FILES yomikataui.rc DESTINATION  ${DATA_INSTALL_DIR}/${CMAKE_PROJECT_NAME})
    kde4_install_icons(${ICON_INSTALL_DIR})
endif()
