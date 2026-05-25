cmake_minimum_required(VERSION 3.5)

project(LhSonarSDKGuiDemo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(USE_WINDOWS)

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

# ================= ������� YOLO �����߼� =================
# 1. �Զ�Ѱ�� OpenCV (���ϵͳû�价����������ȷ���������������ȷ�İ�װ·��)
set(OpenCV_DIR "D:/Cpp_Libraries/opencv-4.12.0-win/opencv/build")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# 2. ONNX Runtime ·������ (���ֱ������������Ժ��޸�)
set(ONNX_ROOT "D:/Cpp_Libraries/onnxruntime-win-x64-gpu-1.20.0")
include_directories(${ONNX_ROOT}/include)
link_directories(${ONNX_ROOT}/lib)
# =======================================================

# add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
add_executable(${PROJECT_NAME} #[[WIN32]] ${PROJECT_SOURCES})


# ================= ͳһ���ӿ����� =================
# ��һ�������еĿ�һ�������ӣ��������
target_link_libraries(${PROJECT_NAME} PRIVATE
    # 1. �������ӵ� Qt ��
    Qt${QT_VERSION_MAJOR}::Widgets 
    Qt${QT_VERSION_MAJOR}::Network
    
    # 2. ���� SDK (���� Debug �� Release)
    debug LhForwardSDK_X64d
    optimized LhForwardSDK_X64
    
    # 3. OpenCV �� ONNX (ʹ�� CMake �ҵ��ı���)
    ${OpenCV_LIBS}
    onnxruntime
    
    # 4. Windows ����� (��ֹ���Ӵ���)
    ws2_32
)
# =================================================