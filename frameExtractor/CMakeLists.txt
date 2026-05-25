cmake_minimum_required(VERSION 3.5)

project(FrameExtractor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui)

# SDK includes (relative to frameExtractor dir)
include_directories(${PROJECT_SOURCE_DIR}/../sdkDemo/LhForwardSDK/include)
include_directories(${PROJECT_SOURCE_DIR}/../sdkDemo/common)
link_directories(${PROJECT_SOURCE_DIR}/../sdkDemo/LhForwardSDK/bin)

add_executable(FrameExtractor main.cpp)

target_link_libraries(FrameExtractor debug LhForwardSDK_X64d)
target_link_libraries(FrameExtractor optimized LhForwardSDK_X64)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_definitions(-DUSE_LINUX=1)
    target_link_libraries(FrameExtractor Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui -lpthread)
else()
    target_link_libraries(FrameExtractor ws2_32 Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui)
    add_definitions(-DUSE_WINDOWS=1)
endif()

# Console app, not GUI
set_target_properties(FrameExtractor PROPERTIES
    WIN32_EXECUTABLE FALSE
)
