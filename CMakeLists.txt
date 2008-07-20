project(yomikata)

# KDE4 dependency
find_package(KDE4 REQUIRED)
include_directories(${KDE4_INCLUDES})
include(KDE4Defaults)

# Build options
set(CMAKE_CXX_FLAGS "-Werror ${CMAKE_CXX_FLAGS}")

# Source files
set(yomikata_SRCS
    main.cpp
)

# Build steps
kde4_add_kcfg_files(yomikata_SRCS settings.kcfgc)
kde4_add_executable(yomikata ${yomikata_SRCS})
target_link_libraries(yomikata ${KDE4_KDEUI_LIBS})

# Unit testing
add_subdirectory(test)

# Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    message(STATUS "Doxygen found, 'doc' target enabled")
    configure_file(Doxyfile.config.doxy ${CMAKE_BINARY_DIR}/Doxyfile)
    add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile DEPENDS ${CMAKE_BINARY_DIR}/Doxyfile COMMENT "Building doc")
else(DOXYGEN_FOUND)
    message(STATUS "Doxygen found, 'doc' target disabled")
endif(DOXYGEN_FOUND)

# Install files
install(TARGETS yomikata DESTINATION ${BIN_INSTALL_DIR})
install(FILES yomikata.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR})
install(FILES yomikata.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES yomikataui.rc DESTINATION  ${DATA_INSTALL_DIR}/yomikata)
kde4_install_icons(${ICON_INSTALL_DIR})
