project(yomikata)

# Source files
set(yomikata_SRCS
    main.cpp
    mainwindow.cpp
    mainwindowkde.cpp
    mainwindowqt.cpp

    debug.cpp

    book.cpp
    revision.cpp
)

# Unit test files
set(test_SRCS
    test.cpp
)

# Unit tests
set(test_CLASSES
    book
)

# TODO add install commands, and make an nsis script that uses the files
#  in the w/e install dir
# TODO make kde optional
# TODO install into the build directory by default
# TODO add clean_doc and clean_lcov to the clean target

# Minimum CMake version
cmake_minimum_required(VERSION 2.6)

# Options
include(cmake/Options.cmake)

# KDE4 dependency
find_package(KDE4 REQUIRED)
include_directories(${KDE4_INCLUDES})
include(KDE4Defaults)

# Development options
if(DEV_MODE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    set(CMAKE_INSTALL_PREFIX /usr
        CACHE STRING
        "Install p!ath prefix, prepended onto install directories.")
endif()

# Unit test build
if(UNIT_TESTING)
    include(cmake/UnitTesting.cmake)
endif()

# Code coverage build
if(CODE_COVERAGE)
    include(cmake/CodeCoverage.cmake)
endif()

# Build steps
kde4_add_kcfg_files(yomikata_SRCS settings.kcfgc)
kde4_add_executable(yomikata ${yomikata_SRCS})
target_link_libraries(yomikata ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${QT_QTTEST_LIBRARY}
    ${COVERAGE_LIBRARY})
set_target_properties(yomikata PROPERTIES COMPILE_FLAGS ${COVERAGE_FLAGS})

# Doxygen
include(cmake/Doxygen.cmake)

# Install files
install(TARGETS yomikata DESTINATION ${BIN_INSTALL_DIR})
install(FILES yomikata.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR})
install(FILES yomikata.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES yomikataui.rc DESTINATION  ${DATA_INSTALL_DIR}/${CMAKE_PROJECT_NAME})
kde4_install_icons(${ICON_INSTALL_DIR})
