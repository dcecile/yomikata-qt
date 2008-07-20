project(yomikata)
find_package(KDE4 REQUIRED)
include_directories(${KDE4_INCLUDES})
include(KDE4Defaults)

set(yomikata_SRCS
  main.cpp
  yomikata.cpp
  pageloader.cpp
  filetypes.cpp
  filedecodejob.cpp
)

kde4_automoc(${yomikata_SRCS})

kde4_add_executable(yomikata ${yomikata_SRCS})

target_link_libraries(yomikata ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_THREADWEAVER_LIBRARY})

install(TARGETS yomikata DESTINATION ${BIN_INSTALL_DIR})
install(FILES yomikataui.rc
         DESTINATION  ${DATA_INSTALL_DIR}/yomikata)
