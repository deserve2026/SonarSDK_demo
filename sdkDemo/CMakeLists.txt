cmake_minimum_required(VERSION 3.5)

project(LhSonarSDKGuiDemo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

include_directories(${PROJECT_SOURCE_DIR}/LhForwardSDK/include)
link_directories(${PROJECT_SOURCE_DIR}/LhForwardSDK/bin)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        common/CircleBuffer.cpp
        net/UdpReceiver.cpp
        Processing.cpp
        FileRead.cpp
        FileSave.cpp
        DeviceInfo.h
        AvoidCollision.cpp
        AvoidCollision.h
)

# ================= YOLO ÒÀÀµ£º°üº¬Ä¿Â¼ =================
include_directories("D:/Cpp_Libraries/opencv-4.12.0-win/opencv/build/include") 
include_directories("D:/Cpp_Libraries/onnxruntime-win-x64-gpu-1.20.0/include")
# =======================================================

# ================= YOLO ÒÀÀµ£º¿âÄ¿Â¼ =================
link_directories("D:/Cpp_Libraries/opencv-4.12.0-win/opencv/build/x64/vc16/lib")
link_directories("D:/Cpp_Libraries/onnxruntime-win-x64-gpu-1.20.0/lib")
# =====================================================

# add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
add_executable(${PROJECT_NAME} #[[WIN32]] ${PROJECT_SOURCES})

# Add third-party libraries
target_link_libraries(${PROJECT_NAME} debug LhForwardSDK_X64d #[[libpng16_staticd zlibstaticd]])
target_link_libraries(${PROJECT_NAME} optimized LhForwardSDK_X64 #[[libpng16_static zlibstatic]])

set_target_properties(LhSonarSDKGuiDemo PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_definitions(-DUSE_LINUX=1)
    add_definitions(-std=c++17)
    target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network -lpthread)
else()
    target_link_libraries(${PROJECT_NAME} ws2_32 Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network opencv_world4120.lib onnxruntime.lib)
    # add_definitions(-DDLL_EXPORTS)
    add_definitions(-DUSE_WINDOWS=1)
endif()

install(TARGETS LhSonarSDKGuiDemo
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# if(QT_VERSION_MAJOR EQUAL 6)
#     qt_finalize_executable(LhSonarSDKGuiDemo)
# endif()
