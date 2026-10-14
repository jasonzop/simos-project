cmake_minimum_required(VERSION 3.20)

project(
    SimOS
    VERSION 1.0.0
    DESCRIPTION "A deterministic operating-system simulator for processes, disks, and virtual memory"
    LANGUAGES CXX
)

add_library(simos src/SimOS.cpp)
add_library(SimOS::SimOS ALIAS simos)

target_include_directories(
    simos
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(simos PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(simos PRIVATE /W4 /permissive-)
else()
    target_compile_options(simos PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(simos_demo examples/demo.cpp)
target_link_libraries(simos_demo PRIVATE SimOS::SimOS)

include(CTest)
if(BUILD_TESTING)
    add_executable(simos_tests tests/simos_tests.cpp)
    target_link_libraries(simos_tests PRIVATE SimOS::SimOS)

    if(MSVC)
        target_compile_options(simos_tests PRIVATE /W4 /permissive-)
    else()
        target_compile_options(simos_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME simos_tests COMMAND simos_tests)
endif()
