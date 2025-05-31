cmake_minimum_required(VERSION 3.16)
project(krunner_fzfrunner)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 设置模块路径
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

# 添加必要的依赖
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(KF6 REQUIRED COMPONENTS 
    Runner 
    I18n 
    Config
    Service
    KIO
    Notifications
)
find_package(Plasma REQUIRED)

# 手动设置安装路径
set(KDE_INSTALL_PLUGINDIR "lib/x86_64-linux-gnu/plugins")
set(KDE_INSTALL_KSERVICES6DIR "share/kservices6")

# 检查变量
message(STATUS "KDE_INSTALL_PLUGINDIR = ${KDE_INSTALL_PLUGINDIR}")
message(STATUS "KDE_INSTALL_KSERVICES6DIR = ${KDE_INSTALL_KSERVICES6DIR}")

# 设置安装路径
set(PLUGIN_INSTALL_DIR "${KDE_INSTALL_PLUGINDIR}/kf6/krunner")
set(SERVICES_INSTALL_DIR "${KDE_INSTALL_KSERVICES6DIR}")
set(CONFIG_INSTALL_DIR "$ENV{HOME}/.config")
set(EXTENDS_INSTALL_DIR "$ENV{HOME}/.local/share/krunner-fzf/extends")

# 将扩展脚本路径定义为编译时常量
add_compile_definitions(FZF_EXTENDS_DIR="${EXTENDS_INSTALL_DIR}")

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
        KF6::Runner
        KF6::I18n
        KF6::ConfigCore
        KF6::Service
        KF6::KIOCore
        KF6::KIOGui
        KF6::Notifications
        Plasma::Plasma
)

# 安装插件
install(TARGETS krunner_fzfrunner DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/krunner)
install(FILES metadata.json DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/krunner)
install(FILES org.kde.krunner.fzfrunner.desktop DESTINATION ${KDE_INSTALL_KSERVICES6DIR})

# 安装配置文件
install(FILES config/krunner-fzf-settings DESTINATION ${CONFIG_INSTALL_DIR})

# 安装扩展脚本
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
