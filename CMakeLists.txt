cmake_minimum_required(VERSION 3.20)

project(simple_notepad_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets PrintSupport)

set(NOTEPAD_SOURCES
    main.cpp
    autosave_manager.cpp
    main_window.cpp
    text_edit.cpp
    spell_checker.cpp
    spell_checker_highlighter.cpp
    find_replace_dialog.cpp
    word_frequency_dialog.cpp
    find_replace_dialog.ui
    word_frequency_dialog.ui
)

qt_add_executable(notepad ${NOTEPAD_SOURCES})

target_link_libraries(notepad PRIVATE Qt6::Core Qt6::Widgets Qt6::PrintSupport)

enable_testing()

add_executable(notepad_core_tests
    tests/notepad_core_tests.cpp
    autosave_manager.cpp
    spell_checker.cpp
)

target_include_directories(notepad_core_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(notepad_core_tests PRIVATE Qt6::Core)

add_test(NAME notepad_core_tests COMMAND notepad_core_tests)
