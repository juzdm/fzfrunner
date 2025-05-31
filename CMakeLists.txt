cmake_minimum_required(VERSION 3.16)
project(fzfrunner)

find_package(ECM REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)

find_package(KF5I18n REQUIRED)
find_package(KF5Runner REQUIRED)
find_package(KF5KIO REQUIRED)
find_package(KF5Notifications REQUIRED)
find_package(KF5Plasma REQUIRED COMPONENTS Plasma)

# 尝试安装配置文件
set(CONFIG_INSTALL_DIR "$ENV{HOME}/.config" CACHE PATH "配置文件安装路径")
install(FILES config/krunner-fzf-settings
    DESTINATION ${CONFIG_INSTALL_DIR}
    OPTIONAL
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

# 如果安装配置文件失败，输出警告信息
if(NOT EXISTS "${CONFIG_INSTALL_DIR}/krunner-fzf-settings")
    message(WARNING "配置文件未能安装到 ${CONFIG_INSTALL_DIR}/krunner-fzf-settings\n"
                   "请手动复制配置文件或使用默认配置")
endif()

# 定义扩展脚本安装路径
set(EXTENDS_INSTALL_DIR "$ENV{HOME}/.local/share/krunner-fzf/extends" 
    CACHE PATH "扩展脚本安装路径")

# 确保目录存在
install(DIRECTORY DESTINATION ${EXTENDS_INSTALL_DIR})

# 安装扩展脚本
install(FILES 
    extends/fzf_find_files.sh
    extends/fzf_find_files_under.sh
    extends/terminal_open.sh
    extends/sing-box_switch.sh
    extends/tmux_session.sh
    extends/vscode_recent.sh
    extends/aosp-find-repo.sh
    extends/fzf_find_repos.sh
    DESTINATION ${EXTENDS_INSTALL_DIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)

# 将扩展脚本路径定义为编译时常量
add_compile_definitions(FZF_EXTENDS_DIR="${EXTENDS_INSTALL_DIR}")

add_library(krunner_fzfrunner MODULE
    src/CommandRunner.cpp
    src/ConfigManager.cpp
    src/ResultHandler.cpp
    src/ScriptBuilder.cpp
    src/CustomeActionCmd.cpp
)

target_link_libraries(krunner_fzfrunner
    KF5::Runner
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::Notifications
    KF5::Plasma
)

install(TARGETS krunner_fzfrunner
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/krunner
)
install(FILES metadata.json
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/krunner
)
install(FILES org.kde.krunner.fzfrunner.desktop
    DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)
