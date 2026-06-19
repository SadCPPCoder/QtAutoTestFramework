cmake_minimum_required(VERSION 3.16)
project(DemoApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Add the library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cpp-server-lib ${CMAKE_CURRENT_BINARY_DIR}/lib)

find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2)

# QML resources
qt_add_resources(QML_RESOURCES qml/qml.qrc)

# Executable
qt_add_executable(${PROJECT_NAME}
    src/main.cpp
    ${QML_RESOURCES}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
    QtAutoTestServer
)
