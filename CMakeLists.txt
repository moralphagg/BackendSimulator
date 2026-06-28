cmake_minimum_required(VERSION 3.19)
project(BackendSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(BackendSimulator
    WIN32 MACOSX_BUNDLE
    main.cpp
    core/GameState.h
    core/GameState.cpp
    core/SkillSystem.h
    core/SkillSystem.cpp
    core/ProjectQueue.h
    core/ProjectQueue.cpp
    core/CheatEngine.h
    core/CheatEngine.cpp
    core/SaveManager.h
    core/SaveManager.cpp
    core/CareerData.h
    core/CareerData.cpp
    core/CareerTypes.h
    core/CareerTypes.cpp
    core/EventSystem.h
    core/EventSystem.cpp
    controller/GameController.h
    controller/GameController.cpp
    ui/MainWindow.h
    ui/MainWindow.cpp
    ui/StatusPanel.h
    ui/StatusPanel.cpp
    ui/SkillsPanel.h
    ui/SkillsPanel.cpp
    ui/ProjectsPanel.h
    ui/ProjectsPanel.cpp
    ui/TerminalWidget.h
    ui/TerminalWidget.cpp
    ui/CommandsPanel.h
    ui/CommandsPanel.cpp
    ui/HelpPanel.h
    ui/HelpPanel.cpp
)

target_link_libraries(BackendSimulator PRIVATE Qt6::Core Qt6::Widgets)

install(TARGETS BackendSimulator
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)