if(WIN32 AND NOT UNIX)
# Do regular looking Windows folders
    set(BIN_INSTALL_DIR bin)
    set(CONFIG_INSTALL_DIR config)
    set(DATA_INSTALL_DIR data)
    set(PROJECT_CONFIG_DIR ${CONFIG_INSTALL_DIR})
    set(PROJECT_DATA_DIR ${DATA_INSTALL_DIR})
else()
# Standard UNIX directories
    set(BIN_INSTALL_DIR bin)
    set(CONFIG_INSTALL_DIR share/config)
    set(DATA_INSTALL_DIR share/apps)
    set(PROJECT_CONFIG_DIR ${CONFIG_INSTALL_DIR}/${PROJECT_NAME})
    set(PROJECT_DATA_DIR ${DATA_INSTALL_DIR}/${PROJECT_NAME})
endif()

# Relative paths for runtime
if(DEV_MODE)
    # Running from build directory
    file(RELATIVE_PATH RELATIVE_DATA_DIR ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/data)
else()
    # Running from install directory
    file(RELATIVE_PATH RELATIVE_DATA_DIR /${BIN_INSTALL_DIR} /${PROJECT_DATA_DIR})
endif()

add_definitions("-DRELATIVE_DATA_DIR=\"${RELATIVE_DATA_DIR}\"")
