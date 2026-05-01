include(CPack)

set(CPACK_PACKAGE_NAME        "Commitly")
set(CPACK_PACKAGE_VENDOR      "VOMLabs")
set(CPACK_PACKAGE_VERSION     "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION "Commitly — Native Git Client")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/vomlabs/commitly")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME    "Commitly")
    set(CPACK_NSIS_PACKAGE_NAME    "Commitly")
    set(CPACK_NSIS_INSTALL_ROOT    "$PROGRAMFILES64")
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Commitly.lnk' '$INSTDIR\\\\bin\\\\commitly.exe'"
    )
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Commitly.lnk'"
    )
elseif(APPLE)
    set(CPACK_GENERATOR      "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "Commitly")
elseif(UNIX)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "VOMLabs <contact@vomlabs.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION    "vcs")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS    "libgit2-1.1, libssl3")
endif()
