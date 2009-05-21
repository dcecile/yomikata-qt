# Install Windows dependencies
if(WIN32 AND NOT UNIX)
    # MinGW multithreading
    find_library(MINGWM10_DLL "mingwm10.dll")

    # Qt
    find_library(QT_QTCORE4_DLL "QtCore4.dll")
    find_library(QT_QTGUI4_DLL "QtGui4.dll")

    if(QT_USE_QTHELP)
        find_library(QT_QTHELP_DLL "QtHelp4.dll")
    endif()
    if(QT_USE_QTNETWORK)
        find_library(QT_QTNETWORK_DLL "QtNetwork4.dll")
        find_library(OPENSSL1_DLL "libssl32.dll")
        find_library(OPENSSL2_DLL "ssleay32.dll")
        find_library(OPENSSL3_DLL "libeay32.dll")
    endif()
    if(QT_USE_QTOPENGL)
        find_library(QT_QTOPENGL4_DLL "QtOpenGL4.dll")
    endif()
    if(QT_USE_QTSCRIPT)
        find_library(QT_QTSCRIPT_DLL "QtScript4.dll")
    endif()
    if(QT_USE_QTSQL)
        find_library(QT_QTSQL_DLL "QtSql4.dll")
    endif()
    if(QT_USE_QTSVG)
        find_library(QT_QTSVG_DLL "QtSvg4.dll")
    endif()
    if(QT_USE_QTTEST)
        find_library(QT_QTTEST_DLL "QtTest4.dll")
    endif()
    if(QT_USE_QTWEBKIT)
        find_library(QT_QTWEBKIT_DLL "QtWebKit4.dll")
    endif()
    if(QT_USE_QTXML)
        find_library(QT_QTXML_DLL "QtXml4.dll")
    endif()
    if(QT_USE_QTXMLPATTERNS)
        find_library(QT_QTXMLPATTERNS_DLL "QtXmlPatterns4.dll")
    endif()

    install(FILES ${MINGWM10_DLL}
        ${QT_QTCORE4_DLL} ${QT_QTGUI4_DLL}
        ${QT_QTHELP_DLL} ${QT_QTNETWORK_DLL} ${QT_QTOPENGL4_DLL}
            ${QT_QTSCRIPT_DLL} ${QT_QTSQL_DLL} ${QT_QTSVG_DLL} ${QT_QTTEST_DLL}
            ${QT_QTWEBKIT_DLL} ${QT_QTXML_DLL} ${QT_QTXMLPATTERNS_DLL}  
            ${OPENSSL1_DLL} ${OPENSSL2_DLL} ${OPENSSL3_DLL}
        DESTINATION ${BIN_INSTALL_DIR})
endif()

if(WIN32 AND NOT UNIX)
    # Use a custom NSI template (for auto-uninstall)
    set(CPACK_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
endif()

include(CPack)
