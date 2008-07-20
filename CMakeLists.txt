project( yomikata )
find_package( KDE4 REQUIRED )
include_directories( ${KDE4_INCLUDES} )
include( KDE4Defaults )

set( CMAKE_CXX_FLAGS "-Werror ${CMAKE_CXX_FLAGS}" )

set( yomikata_SRCS
    main.cpp
    yomikata.cpp

    oyabun.cpp

    source.cpp
    fileclassifier.cpp

    filelist.cpp
    lister.cpp
    archivelister.cpp

    book.cpp
    pageconductor.cpp
    planner.cpp
    cache.cpp
    cacheentry.cpp
    cachemanager.cpp
    page.cpp

    decoder.cpp
    decodethread.cpp
    extractdecodethread.cpp

    depicter.cpp
    lens.cpp
)

kde4_add_kcfg_files( yomikata_SRCS settings.kcfgc )

kde4_add_executable( yomikata ${yomikata_SRCS} )

target_link_libraries( yomikata ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_THREADWEAVER_LIBRARY}) #${QT_QTOPENGL_LIBRARY})

install( TARGETS yomikata DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES yomikata.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES yomikata.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES yomikataui.rc DESTINATION  ${DATA_INSTALL_DIR}/yomikata )
kde4_install_icons( ${ICON_INSTALL_DIR} )
