# Code coverage

# Compile with coverage options
set(COVERAGE_FLAGS "-O0 -fprofile-arcs -ftest-coverage")

# Link to the coverage library
set(COVERAGE_LIBRARY gcov)

# Clean the coverage data files when rebuilding
# TODO only do it when relinked
set(gcov_files "settings.gcda")

foreach(src ${yomikata_SRCS})
    string(REGEX REPLACE ".cpp$" ".gcda" src ${src})
    set(gcov_files ${gcov_files} ${src})
endforeach()

add_custom_target(clean_cov ALL
    COMMAND ${CMAKE_COMMAND} -E remove ${gcov_files}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${CMAKE_PROJECT_NAME}.dir/
    VERBATIM
)

# Set up for generating coverage data
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/lcov)

# Add the lcov target
add_custom_target(lcov
    COMMAND lcov --directory "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${CMAKE_PROJECT_NAME}.dir/" --capture --output-file all.info
    COMMAND lcov -e all.info "${CMAKE_SOURCE_DIR}/*" --output-file project.info
    COMMAND genhtml project.info
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/lcov
    VERBATIM
)
