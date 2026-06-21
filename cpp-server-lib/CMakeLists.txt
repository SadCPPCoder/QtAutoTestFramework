cmake_minimum_required(VERSION 3.16)
project(QtAutoTestServer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Test)

# Platform-specific configuration
if(WIN32)
    # Windows: Use Windows HTTP Server API
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    set(PLATFORM_SOURCES src/HttpServerWin.cpp)
    set(PLATFORM_LIBS Httpapi)
else()
    # Linux: Use libmicrohttpd
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MHD REQUIRED libmicrohttpd)
    set(PLATFORM_SOURCES src/HttpServerLinux.cpp)
    set(PLATFORM_LIBS ${MHD_LIBRARIES})
endif()

# Common source files
set(LIB_SOURCES
    src/QtAutoTestServer.cpp
    ${PLATFORM_SOURCES}
    src/RequestHandler.cpp
    src/ObjectFinder.cpp
    src/EventSender.cpp
    src/ScreenCapture.cpp
    src/JsonHelper.cpp
    src/Logger.cpp
)

set(LIB_HEADERS
    inc/qtautotest/QtAutoTestServer.h
    inc/qtautotest/qtat_export.h
    inc/qtautotest/qtat_config.h
)

# Internal headers
set(INTERNAL_HEADERS
    src/HttpServer.h
    src/RequestHandler.h
    src/ObjectFinder.h
    src/EventSender.h
    src/ScreenCapture.h
    src/JsonHelper.h
    src/Logger.h
)

# Create static library
add_library(QtAutoTestServer STATIC
    ${LIB_SOURCES}
    ${LIB_HEADERS}
    ${INTERNAL_HEADERS}
)

# Include directories
target_include_directories(QtAutoTestServer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/qtautotest
)

# Linux: Add libmicrohttpd include directories
if(NOT WIN32)
    target_include_directories(QtAutoTestServer PRIVATE ${MHD_INCLUDE_DIRS})
endif()

# Link libraries
target_link_libraries(QtAutoTestServer
    PUBLIC
        Qt6::Core
        Qt6::Quick
        Qt6::Test
    PRIVATE
        ${PLATFORM_LIBS}
)

# Compile definitions
target_compile_definitions(QtAutoTestServer PRIVATE
    QTAT_BUILD_STATIC
)
