# QVKTCloudViewer

基于Qt和VTK的点云可视化工具

## 功能

- 从文件中读取点云数据
- 使用VTK渲染显示点云
- 支持交互式相机控制

## 技术栈
- Qt 6
- VTK 9
- CMake
- C++

## 项目结构

```
QVKTCloudViewer/
├── main.cpp           # 程序入口
├── mainwindow.cpp     # 主窗口实现
├── mainwindow.h       # 主窗口头文件
├── mainwindow.ui      # UI界面文件
├── CMakeLists.txt     # CMake配置文件
└── world_points.txt   # 点云数据文件（.gitignore）
```

## 构建方法

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## 使用说明

运行程序后自动加载 `world_points.txt` 文件并渲染点云。

## 依赖项

- Qt 6.x
- VTK 9.x

## 开发环境

- Windows 10/11
- MSVC 或 MinGW
- CMake 3.x+

