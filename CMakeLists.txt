project( yomikata )
find_package( KDE4 REQUIRED )
include_directories( ${KDE4_INCLUDES} )
include( KDE4Defaults )

set( yomikata_SRCS
  main.cpp
  yomikata.cpp
  fileinfo.cpp

  pagecache.cpp
  lister.cpp
  filelister.cpp
  extractlister.cpp
  decodethread.cpp
  filedecodethread.cpp
  extractdecodethread.cpp

  pagedisplay.cpp
  displayscroller.cpp
  simplescroller.cpp
  inertiascroller.cpp
  testwidget.cpp
)

kde4_add_kcfg_files( yomikata_SRCS settings.kcfgc )

kde4_add_executable( yomikata ${yomikata_SRCS} )

target_link_libraries( yomikata ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_THREADWEAVER_LIBRARY} )

install(TARGETS yomikata DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES yomikata.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES yomikata.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES yomikataui.rc DESTINATION  ${DATA_INSTALL_DIR}/yomikata )
kde4_install_icons( ${ICON_INSTALL_DIR} )
