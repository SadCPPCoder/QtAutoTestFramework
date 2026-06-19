cmake_minimum_required(VERSION 3.16)
project(QtAutoTestServer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Windows HTTP Server API
add_definitions(-DWIN32_LEAN_AND_MEAN)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Test)

# 源文件
set(LIB_SOURCES
    src/QtAutoTestServer.cpp
    src/HttpServer.cpp
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

# 内部头文件
set(INTERNAL_HEADERS
    src/HttpServer.h
    src/RequestHandler.h
    src/ObjectFinder.h
    src/EventSender.h
    src/ScreenCapture.h
    src/JsonHelper.h
    src/Logger.h
)

# 创建静态库
add_library(QtAutoTestServer STATIC
    ${LIB_SOURCES}
    ${LIB_HEADERS}
    ${INTERNAL_HEADERS}
)

# 包含目录
target_include_directories(QtAutoTestServer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/qtautotest
)

# 链接库
target_link_libraries(QtAutoTestServer
    PUBLIC
        Qt6::Core
        Qt6::Quick
        Qt6::Test
    PRIVATE
        Httpapi
)

# 编译定义
target_compile_definitions(QtAutoTestServer PRIVATE
    QTAT_BUILD_STATIC
)
