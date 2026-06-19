cmake_minimum_required(VERSION 3.20)

project(BlackBox VERSION 0.1.0 LANGUAGES CXX)
find_package(SQLite3 REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(blackbox
    src/active_window.cpp
    src/activity_segment.cpp
    src/database.cpp
    src/main.cpp
    src/reporter.cpp
    src/text.cpp
    src/recorder.cpp
)

add_executable(blackbox_gui WIN32
    src/active_window.cpp
    src/app_icon.cpp
    src/activity_segment.cpp
    src/database.cpp
    src/gui_main.cpp
    src/main_window.cpp
    src/recorder.cpp
    src/text.cpp
)

include(cTest)

if(BUILD_TESTING)

add_executable(activity_segment_test
    tests/activity_segment_test.cpp
    src/activity_segment.cpp
)

target_compile_features(activity_segment_test PRIVATE cxx_std_20)
target_include_directories(activity_segment_test PRIVATE src)
add_test(NAME activity_segment_test COMMAND activity_segment_test)

add_executable(database_test
    tests/database_test.cpp
    src/database.cpp
    src/text.cpp
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
