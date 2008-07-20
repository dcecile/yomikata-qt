# Unit testing

set(tester_SRCS
    test.cpp
)

enable_testing()

include_directories(${CMAKE_SOURCE_DIR})
kde4_add_executable(tester ${tester_SRCS})
target_link_libraries(tester ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

add_test(tester tester)
