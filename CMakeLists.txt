# Silicon LogiX / SLX Test Report
# Copyright (c) 2026 Marco Pezzullo (Silicon LogiX).
# Author: Marco Pezzullo
# License: Silicon LogiX Evaluation License 1.0. See LICENSE.

# Build the CSV/profile models, PDF exporter, Qt Quick application, and tests.

cmake_minimum_required(VERSION 3.21)

project(SLXTestReport VERSION 0.5.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Quick QuickControls2 Svg)

add_library(slx-domain STATIC
    src/reportdata.cpp
    src/reportdata.h
    src/csvcomposer.cpp
    src/csvcomposer.h
    src/profiledata.cpp
    src/profiledata.h
    src/resultsmodel.cpp
    src/resultsmodel.h
    src/resultsfiltermodel.cpp
    src/resultsfiltermodel.h
)
target_include_directories(slx-domain PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(slx-domain PUBLIC Qt6::Core)

add_library(slx-reporting STATIC
    src/reportcontroller.cpp
    src/reportcontroller.h
    src/pdfexport.cpp
    src/pdfexport.h
)
target_include_directories(slx-reporting PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(slx-reporting PUBLIC slx-domain Qt6::Gui Qt6::Svg)

qt_add_executable(slx-test-report
    src/main.cpp
    README.md
    LICENSE
    THIRD_PARTY_NOTICES.md
    THIRD_PARTY_SOURCE.md
    licenses/Qt-Open-Source-LICENSE.txt
    assets/brand/logo-light.svg
    resources.qrc
)

target_link_libraries(slx-test-report PRIVATE slx-reporting Qt6::Quick Qt6::QuickControls2)
target_compile_definitions(slx-test-report PRIVATE SLX_APP_VERSION="${PROJECT_VERSION}")

foreach(target slx-domain slx-reporting slx-test-report)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            # Keep local checkout paths out of compiler-generated file references.
            target_compile_options(${target} PRIVATE "-ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}=.")
        endif()
    endif()
endforeach()

if(WIN32)
    target_sources(slx-test-report PRIVATE assets/brand/appicon.rc)
    qt_add_executable(slx-icon-maker EXCLUDE_FROM_ALL scripts/make-icon.cpp)
    target_link_libraries(slx-icon-maker PRIVATE Qt6::Gui Qt6::Svg)
endif()

set_target_properties(slx-test-report PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

include(CTest)
if(BUILD_TESTING)
    add_test(NAME app-smoke COMMAND slx-test-report --smoke-test)
    set_tests_properties(app-smoke PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    find_package(Qt6 6.2 COMPONENTS Test QUIET)
    if(Qt6Test_FOUND)
        qt_add_executable(reportdata-tests
            tests/reportdata_tests.cpp
        )
        target_link_libraries(reportdata-tests PRIVATE slx-domain Qt6::Test)
        add_test(NAME reportdata-tests COMMAND reportdata-tests)

        qt_add_executable(resultsfilter-tests
            tests/resultsfilter_tests.cpp
        )
        target_link_libraries(resultsfilter-tests PRIVATE slx-domain Qt6::Test)
        add_test(NAME resultsfilter-tests COMMAND resultsfilter-tests)

        qt_add_executable(workflow-tests
            tests/workflow_tests.cpp
            resources.qrc
        )
        target_link_libraries(workflow-tests PRIVATE slx-reporting Qt6::Test)
        add_test(NAME workflow-tests COMMAND workflow-tests)
        if(WIN32)
            set_tests_properties(workflow-tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")
        else()
            set_tests_properties(workflow-tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
        endif()

        qt_add_executable(profiledata-tests
            tests/profiledata_tests.cpp
        )
        target_link_libraries(profiledata-tests PRIVATE slx-domain Qt6::Test)
        add_test(NAME profiledata-tests COMMAND profiledata-tests)

        qt_add_executable(csvcomposer-tests
            tests/csvcomposer_tests.cpp
        )
        target_link_libraries(csvcomposer-tests PRIVATE slx-domain Qt6::Test)
        add_test(NAME csvcomposer-tests COMMAND csvcomposer-tests)
    endif()
endif()
