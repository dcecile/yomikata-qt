# Unit testing
enable_testing()

# Add the files to the main source files
foreach(src ${test_SRCS})
    set(yomikata_SRCS ${yomikata_SRCS} test/${src})
endforeach()

# For each test class
foreach(class ${test_CLASSES})
    # Add it to the list of files to compile
    set(yomikata_SRCS ${yomikata_SRCS} test/${class}test.cpp)

    # Make a ctest test
    add_test(${class} yomikata ${class})
endforeach()

# Include files from the main directory
include_directories(${CMAKE_SOURCE_DIR})

# Add compiler definition
add_definitions(-DUNIT_TESTING)
