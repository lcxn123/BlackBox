cmake_minimum_required(VERSION 3.20)

project(BlackBox VERSION 0.1.0 LANGUAGES CXX)
find_package(SQLite3 REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(blackbox
    src/cli/main.cpp
    src/core/activity_segment.cpp
    src/core/text.cpp
    src/platform/windows/active_window.cpp
    src/recorder/recorder.cpp
    src/reporting/reporter.cpp
    src/storage/database.cpp
)

add_executable(blackbox_gui WIN32
    src/core/activity_segment.cpp
    src/core/text.cpp
    src/gui/app_icon.cpp
    src/gui/gui_main.cpp
    src/gui/main_window.cpp
    src/gui/main_window_style.cpp
    src/gui/recording_controller.cpp
    src/gui/settings_dialog.cpp
    src/gui/usage_bar_chart.cpp
    src/platform/windows/active_window.cpp
    src/recorder/recorder.cpp
    src/reporting/usage_report.cpp
    src/settings/app_settings.cpp
    src/storage/database.cpp
)

include(cTest)

if(BUILD_TESTING)

add_executable(activity_segment_test
    tests/activity_segment_test.cpp
    src/core/activity_segment.cpp
)

target_compile_features(activity_segment_test PRIVATE cxx_std_20)
target_include_directories(activity_segment_test PRIVATE src)
add_test(NAME activity_segment_test COMMAND activity_segment_test)

add_executable(database_test
    tests/database_test.cpp
    src/core/text.cpp
    src/storage/database.cpp
)

target_compile_features(database_test PRIVATE cxx_std_20)
target_include_directories(database_test PRIVATE src)
target_link_libraries(
    database_test
    PRIVATE
    SQLite::SQLite3)
add_test(NAME database_test COMMAND database_test)

endif()

target_compile_features(blackbox PRIVATE cxx_std_20)
target_compile_features(blackbox_gui PRIVATE cxx_std_20)
target_include_directories(blackbox PRIVATE src)
target_include_directories(blackbox_gui PRIVATE src)

target_compile_definitions(blackbox PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

if(MINGW OR CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(blackbox PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(
    blackbox
    PRIVATE
    user32
    SQLite::SQLite3
)

target_link_libraries(
    blackbox_gui
    PRIVATE
    Qt6::Widgets
    user32
    SQLite::SQLite3
)
