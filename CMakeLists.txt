project(yomikata)

# Source files
set(yomikata_SRCS
    mainwindow.cpp

    viewwidget.cpp
    toolbarwidget.cpp

    steward.cpp
    artificer.cpp
    strategist.cpp
    book.cpp
    archive.cpp
    indexer.cpp
    projector.cpp

    archivelister.cpp
    fileclassification.cpp
    decodethread.cpp
    imagesource.cpp

    scroller.cpp
    loadingsprite.cpp
    pagesprite.cpp

    mean.cpp
    debug.cpp
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
    strategist
)

# TODO install into the build directory by default
# TODO add clean_doc and clean_lcov to the clean target
# TODO enable warnings like KDE does

# Minimum CMake version
cmake_minimum_required(VERSION 2.6)

# Options
include(cmake/Options.cmake)

# Paths
include(cmake/Paths.cmake)

# Dependencies
if(QT_ONLY)
    # Qt-only dependency
    find_package(Qt4 REQUIRED)
    if(UNIT_TESTING)
        set(QT_USE_QTTEST 1)
    endif()
    include(${QT_USE_FILE})
    include_directories(${QT4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})
else()
    # KDE4 dependency
    find_package(KDE4 REQUIRED)
    include_directories(${KDE4_INCLUDES})
    include(KDE4Defaults)
endif()

# Windows 7zip
if(WIN32)
	find_path(SEVENZIP_PATH "7z.exe")
	
	if(DEV_MODE)
		add_definitions('-DWINDOWS_FIRST_PATH="${SEVENZIP_PATH}"')
	else()
		add_definitions('-DWINDOWS_FIRST_PATH="."')
	endif()
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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -O0 -ggdb")
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

# Link flags
if(WIN32 AND NOT UNIX)
    # Don't open a console in windows
    message("test")
    set(LINK_FLAGS "${LINK_FLAGS} -mwindows")
endif()

# EXE icon (modeled off http://svn.berlios.de/svnroot/repos/sim-im/trunk/sim/sim.rc)
if(WIN32)
  if(MINGW)
    # resource compilation for mingw
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/icon_rc.o
                       DEPENDS icon.rc
                       COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR}
                                           -i${CMAKE_CURRENT_SOURCE_DIR}/icon.rc
                                           -o ${CMAKE_CURRENT_BINARY_DIR}/icon_rc.o)
    set(yomikata_SRCS ${yomikata_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/icon_rc.o)
  else()
    set(yomikata_SRCS ${yomikata_SRCS} icon.rc)
  endif()
endif()

if(QT_ONLY)
    # Qt-only build steps
    qt4_automoc(${yomikata_SRCS})
    qt4_add_resources(yomikata_SRCS yomikata.qrc)
    add_executable(yomikata ${yomikata_SRCS})
    target_link_libraries(yomikata ${QT_LIBRARIES} ${COVERAGE_LIBRARY})
	if(CODE_COVERAGE)
	    set_target_properties(yomikata PROPERTIES COMPILE_FLAGS ${COVERAGE_FLAGS})
	endif()
else()
    # KDE build steps
    kde4_add_kcfg_files(yomikata_SRCS settings.kcfgc)
    kde4_add_executable(yomikata ${yomikata_SRCS})
    target_link_libraries(yomikata ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${QT_QTTEST_LIBRARY}
        ${COVERAGE_LIBRARY})
    set_target_properties(yomikata PROPERTIES COMPILE_FLAGS ${COVERAGE_FLAGS})
endif()

# Set flags
#set_target_properties(yomikata PROPERTIES COMPILE_FLAGS ${CXX_FLAGS})
set_target_properties(yomikata PROPERTIES LINK_FLAGS "${CXX_FLAGS} ${LINK_FLAGS}")

# Doxygen
include(cmake/Doxygen.cmake)

if(QT_ONLY)
    # Qt-only install
	install(TARGETS yomikata DESTINATION ${BIN_INSTALL_DIR})
    install(FILES ${SEVENZIP_PATH}/7z.exe  
        DESTINATION ${BIN_INSTALL_DIR})
        set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A liberated and enjoyable comics reading experience.")
	
	set(CPACK_PACKAGE_VENDOR "Yomikata")
	set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.TXT")
	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.TXT")
	set(CPACK_PACKAGE_VERSION_MAJOR "0")
	set(CPACK_PACKAGE_VERSION_MINOR "0")
	set(CPACK_PACKAGE_VERSION_PATCH "1")
	set(CPACK_PACKAGE_INSTALL_DIRECTORY "Yomikata")
	set(CPACK_PACKAGE_EXECUTABLES "yomikata" "Yomikata")
	
	if(WIN32 AND NOT UNIX)
		set(CPACK_NSIS_MODIFY_PATH OFF)
	    set(CPACK_PACKAGE_FILE_NAME "yomikata-setup")
	    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/icon\\\\icon.ico")
	    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/icon\\\\uninstall.ico")
	    #set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/data/icon\\\\install_banner.bmp")
	    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\yomikata.exe")
	    set(CPACK_NSIS_DISPLAY_NAME "Yomikata")
	endif()
	
	include(cmake/Install.cmake)
        
else()
    # KDE install
    install(TARGETS yomikata DESTINATION ${BIN_INSTALL_DIR})
    install(FILES yomikata.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR})
    install(FILES yomikata.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
    install(FILES yomikataui.rc DESTINATION  ${DATA_INSTALL_DIR}/${CMAKE_PROJECT_NAME})
    kde4_install_icons(${ICON_INSTALL_DIR})
endif()
