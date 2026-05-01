# Helper: deploy Qt runtime libraries next to the executable after install.
# Invoked by the top-level CMakeLists install step.

cmake_minimum_required(VERSION 3.25)

if(WIN32)
    find_program(WINDEPLOYQT windeployqt HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT)
        install(CODE "
            execute_process(
                COMMAND \"${WINDEPLOYQT}\" \"$<TARGET_FILE:commitly>\"
                    --release --no-translations --no-system-d3d-compiler
                WORKING_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}/bin\"
            )
        ")
    else()
        message(WARNING "windeployqt not found — Qt libraries will not be bundled.")
    endif()

elseif(APPLE)
    find_program(MACDEPLOYQT macdeployqt HINTS "${Qt6_DIR}/../../../bin")
    if(MACDEPLOYQT)
        install(CODE "
            execute_process(
                COMMAND \"${MACDEPLOYQT}\" \"$<TARGET_FILE_DIR:commitly>/../..\"
                    -always-overwrite
            )
        ")
    else()
        message(WARNING "macdeployqt not found — Qt frameworks will not be bundled.")
    endif()

elseif(UNIX)
    find_program(LINUXDEPLOY linuxdeploy LINUXDEPLOY_PLUGIN_QT linuxdeploy-plugin-qt)
    if(LINUXDEPLOY)
        install(CODE "
            execute_process(
                COMMAND \"${LINUXDEPLOY}\"
                    --executable \"$<TARGET_FILE:commitly>\"
                    --appdir \"\${CMAKE_INSTALL_PREFIX}\"
                    --plugin qt
                    --output appimage
            )
        ")
    else()
        message(WARNING "linuxdeploy not found — Qt libraries will not be bundled on Linux.")
    endif()
endif()
