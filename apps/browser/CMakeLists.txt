# Start of tes3mp change (major)
#
# Ported from Qt5 to Qt6.
#
# 0.8.1 built the browser against Qt5 with explicit QT5_WRAP_CPP / QT5_WRAP_UI /
# QT5_ADD_RESOURCES calls and qt5_use_modules(). 0.51 requires Qt6 (see the
# find_package in the top-level CMakeLists), where qt5_use_modules() is gone and
# the wrap macros are spelled QT_*. Rather than translate macro for macro, this
# follows what apps/launcher does in 0.51: let AUTOMOC and AUTOUIC find the moc
# and .ui work, and link the Qt:: imported targets.
#
# The CMAKE_CXX_STANDARD 14 that used to head this file is gone deliberately.
# Qt6 needs C++17 at minimum and the rest of the tree is well past that; leaving
# it would have quietly downgraded this one directory below what its own
# dependencies require.
# End of tes3mp change (major)

set(BROWSER_UI
        ${CMAKE_SOURCE_DIR}/files/tes3mp/ui/Main.ui
        ${CMAKE_SOURCE_DIR}/files/tes3mp/ui/ServerInfo.ui
        )
set(BROWSER
        main.cpp
        MainWindow.cpp
        ServerModel.cpp
        ServerInfoDialog.cpp
        MySortFilterProxyModel.cpp
        netutils/HTTPNetwork.cpp
        netutils/Utils.cpp
        netutils/QueryClient.cpp
        PingUpdater.cpp
        PingHelper.cpp
        QueryHelper.cpp
        ${CMAKE_SOURCE_DIR}/files/tes3mp/browser.rc
        )

set(BROWSER_HEADER_MOC
        MainWindow.hpp
        ServerModel.hpp
        ServerInfoDialog.hpp
        MySortFilterProxyModel.hpp
        PingUpdater.hpp
        PingHelper.hpp
        QueryHelper.hpp
        )

set(BROWSER_HEADER
        ${BROWSER_HEADER_MOC}
        netutils/HTTPNetwork.hpp
        netutils/Utils.hpp
        netutils/QueryClient.hpp
        Types.hpp
        )

source_group(browser FILES ${BROWSER} ${BROWSER_HEADER})

# Set some platform specific settings
if(WIN32)
    set(GUI_TYPE WIN32)
endif(WIN32)

QT_ADD_RESOURCES(RCC_SRCS ${CMAKE_SOURCE_DIR}/files/launcher/launcher.qrc)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
if(NOT WIN32)
    include_directories(${LIBUNSHIELD_INCLUDE_DIR})
endif(NOT WIN32)

# Main executable
add_executable(tes3mp-browser
        ${GUI_TYPE}
        ${BROWSER}
        ${BROWSER_HEADER}
        ${BROWSER_UI}
        ${RCC_SRCS}
        )

if (WIN32)
    INSTALL(TARGETS tes3mp-browser RUNTIME DESTINATION ".")
endif (WIN32)

target_link_libraries(tes3mp-browser
        ${SDL2_LIBRARY_ONLY}
        ${RakNet_LIBRARY}
        components
        # 0.51 split the Qt-dependent components (Process::ProcessInvoker, which
        # MainWindow uses to launch the client) into their own static library.
        components_qt
        Qt::Widgets
        Qt::Core
        # components_qt's Misc::ScalableIcon renders SVGs, so anything linking it
        # needs Qt::Svg -- apps/launcher links it for the same reason.
        Qt::Svg
        )

set_property(TARGET tes3mp-browser PROPERTY AUTOMOC ON)
set_property(TARGET tes3mp-browser PROPERTY AUTOUIC ON)
set_property(TARGET tes3mp-browser PROPERTY AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/files/tes3mp/ui)

if (BUILD_WITH_CODE_COVERAGE)
    add_definitions (--coverage)
    target_link_libraries(tes3mp-browser gcov)
endif()
