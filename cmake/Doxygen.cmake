# Doxygen

find_package(Doxygen)
if(DOXYGEN_FOUND)
    message(STATUS "Doxygen found, 'doc' target enabled")
    configure_file(Doxyfile.config.doxy ${CMAKE_BINARY_DIR}/Doxyfile)
    add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile DEPENDS ${CMAKE_BINARY_DIR}/Doxyfile COMMENT "Building doc")
else()
    message(STATUS "Doxygen found, 'doc' target disabled")
endif()
