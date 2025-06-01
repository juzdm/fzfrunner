cmake_minimum_required(VERSION 3.16)
project(krunner_fzfrunner)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find ECM
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

# Required dependencies
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets DBus)
find_package(KF6 REQUIRED COMPONENTS 
    Runner 
    I18n 
    Config
    Service
    KIO
    Notifications
)
find_package(Plasma REQUIRED)

# Source files
set(krunner_fzfrunner_SRCS
    src/CommandRunner.cpp
    src/ConfigManager.cpp
    src/ScriptBuilder.cpp
    src/ResultHandler.cpp
    src/CustomeActionCmd.cpp
)

add_library(krunner_fzfrunner MODULE ${krunner_fzfrunner_SRCS})

target_include_directories(krunner_fzfrunner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    /usr/include/KF6
    /usr/include/KF6/KService
    /usr/include/KF6/KIOCore
    /usr/include/KF6/KIOGui
    /usr/include/KF6/KIOWidgets
    ${Plasma_INCLUDE_DIRS}
)

target_link_libraries(krunner_fzfrunner
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::DBus
        KF6::Runner
        KF6::I18n
        KF6::ConfigCore
        KF6::Service
        KF6::KIOCore
        KF6::KIOGui
        KF6::Notifications
        Plasma::Plasma
)

# Installation paths
install(TARGETS krunner_fzfrunner DESTINATION ${CMAKE_INSTALL_LIBDIR}/qt6/plugins/kf6/krunner)
install(FILES metadata.json DESTINATION ${CMAKE_INSTALL_LIBDIR}/qt6/plugins/kf6/krunner)
install(FILES org.kde.krunner.fzfrunner.desktop DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kservices6)

# Config and extends installation
set(CONFIG_INSTALL_DIR "$ENV{HOME}/.config")
set(EXTENDS_INSTALL_DIR "$ENV{HOME}/.local/share/krunner-fzf/extends")

# Add extends directory path as compile definition
add_compile_definitions(FZF_EXTENDS_DIR="${EXTENDS_INSTALL_DIR}")

# Install config file
install(FILES config/krunner-fzf-settings DESTINATION ${CONFIG_INSTALL_DIR})

# Install extends scripts
install(PROGRAMS
    extends/fzf_find_files.sh
    extends/fzf_find_files_under.sh
    extends/terminal_open.sh
    extends/sing-box_switch.sh
    extends/tmux_session.sh
    extends/vscode_recent.sh
    extends/aosp-find-repo.sh
    extends/fzf_find_repos.sh
    DESTINATION ${EXTENDS_INSTALL_DIR}
)
