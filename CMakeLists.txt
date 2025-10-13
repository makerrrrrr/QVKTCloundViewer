cmake_minimum_required(VERSION 3.16)
project(QVKTCloudViewer)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)

# set设置VTK安装目录
set(VTK_DIR "D:/VTK-9.5.2/release/lib/cmake/vtk-9.5")
find_package(VTK REQUIRED COMPONENTS
    CommonCore
    CommonDataModel
    CommonColor
    RenderingCore
    RenderingOpenGL2
    RenderingUI
    InteractionStyle
    FiltersCore
    FiltersGeneral
    GUISupportQt
)

# 启用 Qt 的 MOC、UIC、RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 添加可执行文件
add_executable(QVKTCloudViewer
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

# 链接 Qt6 库
target_link_libraries(QVKTCloudViewer
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
)

# 链接 VTK 库（根据代码使用到的模块补全）
target_link_libraries(QVKTCloudViewer
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::CommonColor
    VTK::RenderingCore
    VTK::RenderingOpenGL2
    VTK::RenderingUI
    VTK::InteractionStyle
    VTK::FiltersCore
    VTK::FiltersGeneral
    VTK::GUISupportQt
)

# 使用 VTK 目标自带的包含目录即可，无需手动追加路径
target_include_directories(QVKTCloudViewer PRIVATE ${VTK_INCLUDE_DIRS})
