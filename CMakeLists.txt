cmake_minimum_required(VERSION 3.20)

project(simple_notepad_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets PrintSupport)

set(NOTEPAD_APP_SOURCES
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

qt_add_executable(notepad main.cpp ${NOTEPAD_APP_SOURCES})

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

add_executable(notepad_gui_smoke_tests
    tests/notepad_gui_smoke_tests.cpp
    ${NOTEPAD_APP_SOURCES}
)

target_include_directories(notepad_gui_smoke_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(notepad_gui_smoke_tests PRIVATE Qt6::Core Qt6::Widgets Qt6::PrintSupport)

add_test(NAME notepad_gui_smoke_tests COMMAND notepad_gui_smoke_tests)
set_tests_properties(notepad_gui_smoke_tests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
