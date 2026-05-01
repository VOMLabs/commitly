cmake_minimum_required(VERSION 3.25)
project(commitly VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ── Dependencies ──────────────────────────────────────────────────────────────
include(FetchContent)

# Corrosion: CMake + Cargo integration
FetchContent_Declare(
    Corrosion
    GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
    GIT_TAG        v0.5.0
)
FetchContent_MakeAvailable(Corrosion)

# Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)
qt_standard_project_setup()

# ── Rust core crate ───────────────────────────────────────────────────────────
corrosion_import_crate(
    MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/crates/commitly-core/Cargo.toml
    CRATES        commitly-core
)

# CXX-generated bridge headers are placed by corrosion into the build dir
set(CXX_BRIDGE_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/corrosion_generated/cxxbridge")

# ── C++ sources ───────────────────────────────────────────────────────────────
set(COMMITLY_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/repository_list_panel.cpp
    src/toolbar_widget.cpp
    src/changes_panel.cpp
    src/history_panel.cpp
    src/diff_viewer.cpp
    src/commit_widget.cpp
    src/branch_panel.cpp
    src/clone_dialog.cpp
    src/preferences_dialog.cpp
    src/sign_in_dialog.cpp
    src/stash_panel.cpp
)

set(COMMITLY_HEADERS
    src/mainwindow.h
    src/repository_list_panel.h
    src/toolbar_widget.h
    src/changes_panel.h
    src/history_panel.h
    src/diff_viewer.h
    src/commit_widget.h
    src/branch_panel.h
    src/clone_dialog.h
    src/preferences_dialog.h
    src/sign_in_dialog.h
    src/stash_panel.h
    src/git_worker.h
    src/theme.h
)

qt_add_executable(commitly
    ${COMMITLY_SOURCES}
    ${COMMITLY_HEADERS}
)

target_include_directories(commitly PRIVATE
    src/
    ${CXX_BRIDGE_INCLUDE_DIR}
)

target_link_libraries(commitly PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    commitly-core
)

# ── Go compiler build ─────────────────────────────────────────────────────────
find_program(GO_EXECUTABLE go REQUIRED)

set(COMMITLYC_OUTPUT "${CMAKE_BINARY_DIR}/commitlyc${CMAKE_EXECUTABLE_SUFFIX}")
set(COMPILER_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../compiler")

add_custom_command(
    OUTPUT  ${COMMITLYC_OUTPUT}
    COMMAND ${GO_EXECUTABLE} build -o ${COMMITLYC_OUTPUT} .
    WORKING_DIRECTORY ${COMPILER_SRC_DIR}
    COMMENT "Building commitlyc (Go compiler)"
    VERBATIM
)
add_custom_target(commitlyc ALL DEPENDS ${COMMITLYC_OUTPUT})

# ── Install / Deploy ──────────────────────────────────────────────────────────
include(cmake/QtDeploy.cmake)
include(cmake/CPackConfig.cmake)

install(TARGETS commitly DESTINATION bin)
install(PROGRAMS ${COMMITLYC_OUTPUT} DESTINATION bin)
