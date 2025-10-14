#include "mainwindow.h"
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkCamera.h>
#include <vtkNamedColors.h>
#include <vtkUnsignedCharArray.h>
#include <QVTKOpenGLNativeWidget.h>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // ========================================================================
    // 第一步：初始化Qt界面
    // ========================================================================
    ui->setupUi(this);  // 加载UI文件，创建所有Qt控件

    // ========================================================================
    // 第二步：建立VTK图形管线基础（核心三要素）
    // ========================================================================
    // 图形管线结构：QVTKWidget ← RenderWindow ← Renderer ← Actor
    
    renderer = vtkSmartPointer<vtkRenderer>::New();
    
    // 2.2 获取渲染窗口（管理OpenGL上下文，连接Qt和VTK的桥梁）
    // ui->vtkWidget 是在mainwindow.ui中定义的QVTKOpenGLNativeWidget控件
    renderWindow = ui->vtkWidget->renderWindow();
    
    // 2.3 将渲染器添加到渲染窗口（建立连接）
    // 一个窗口可以包含多个渲染器，实现分屏显示
    renderWindow->AddRenderer(renderer);

    // ========================================================================
    // 第三步：设置用户交互方式
    // ========================================================================
    // 获取交互器（处理鼠标、键盘事件）
    auto interactor = renderWindow->GetInteractor();
    
    // 设置交互风格为轨迹球相机（允许用鼠标旋转、缩放、平移）
    // - 左键拖动：旋转视角
    // - 中键/右键拖动：平移视角
    // - 滚轮：缩放
    auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    interactor->SetInteractorStyle(style);

    // ========================================================================
    // 第四步：执行数据管线（从文件读取点云数据）
    // ======================================================================== // 数据管线流程：文件 → vtkPoints → vtkPolyData → vtkFilter → 处理后的PolyData
    // 尝试多个可能的文件路径
    QString filename = "world_points_with_color.txt";
    if (!QFile::exists(filename)) {
        // 如果在当前目录找不到，尝试项目根目录
        filename = "../../world_points_with_color.txt";
        qDebug() << "尝试项目根目录:" << filename;
    }
    if (!QFile::exists(filename)) {
        // 如果还找不到，使用绝对路径
        filename = "D:/QTProject/QVKTCloudViewer/world_points_with_color.txt";
        qDebug() << "尝试绝对路径:" << filename;
    }
    
    auto pointCloud = createPointCloudFromFile(filename); // 从文件读取点云数据
    
    // ========================================================================
    // 第五步：执行图形管线（将数据转换为可视化对象）
    // ========================================================================
    // 图形管线流程：PolyData → Mapper → Actor → Renderer
    auto actor = createPointCloudActor(pointCloud);  // 创建Actor（包含Mapper和属性）
    renderer->AddActor(actor);                       // 将Actor添加到场景中

    // ========================================================================
    // 第六步：场景美化设置
    // ========================================================================
    // 设置背景颜色为灰色
    vtkSmartPointer<vtkNamedColors> colors = vtkSmartPointer<vtkNamedColors>::New();
    renderer->SetBackground(colors->GetColor3d("Gray").GetData());

    // ========================================================================
    // 第七步：相机设置（调整观察视角）
    // ========================================================================
    renderer->ResetCamera();                    // 自动调整相机以显示所有物体
    renderer->GetActiveCamera()->Azimuth(30);   // 水平旋转30度
    renderer->GetActiveCamera()->Elevation(30); // 垂直旋转30度
    renderer->ResetCameraClippingRange();       // 重置裁剪范围

    // ========================================================================
    // 第八步：执行渲染（将所有设置绘制到屏幕）
    // ========================================================================
    renderWindow->Render();  // 触发渲染管线，显示结果
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ============================================================================
// 数据管线函数：从文件读取点云数据（包含颜色信息）
// 流程：文件 → vtkPoints + vtkColors → vtkPolyData → vtkFilter → 可渲染数据
// ============================================================================
vtkSmartPointer<vtkPolyData> MainWindow::createPointCloudFromFile(const QString& filename)
{
    // ------------------------------------------------------------------------
    // 数据管线第1步：从文件读取原始数据（坐标 + 颜色）
    // ------------------------------------------------------------------------
    auto points = vtkSmartPointer<vtkPoints>::New();
    auto colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colors->SetNumberOfComponents(3);  // RGB三个分量
    colors->SetName("Colors");         // 设置颜色数组名称
    
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件:" << filename;
        return nullptr;
    }
    
    QTextStream in(&file);
    int pointCount = 0;
    bool firstLine = true;  // 标记是否为第一行（标题行）
    
    // 逐行读取文件中的点坐标和颜色
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;  // 跳过空行
        
        // 跳过第一行标题（X Y Z R G B）
        if (firstLine) {
            firstLine = false;
            if (line.contains("X") && line.contains("Y") && line.contains("Z")) {
                qDebug() << "检测到标题行，已跳过:" << line;
                continue;
            }
        }
        
        QStringList values = line.split(' ', Qt::SkipEmptyParts);
        if (values.size() != 6) {
            qDebug() << "跳过格式错误的行 (需要6个值):" << line;
            continue;
        }
        
        // 解析x, y, z坐标
        bool ok;
        double x = values[0].toDouble(&ok);
        if (!ok) continue;
        double y = values[1].toDouble(&ok);
        if (!ok) continue;
        double z = values[2].toDouble(&ok);
        if (!ok) continue;
        
        // 解析R, G, B颜色值（注意：文件中的颜色值是浮点数）
        double r_double = values[3].toDouble(&ok);
        if (!ok) continue;
        double g_double = values[4].toDouble(&ok);
        if (!ok) continue;
        double b_double = values[5].toDouble(&ok);
        if (!ok) continue;
        
        // 转换为整数颜色值
        int r = static_cast<int>(r_double);
        int g = static_cast<int>(g_double);
        int b = static_cast<int>(b_double);
        
        // 将点插入到points容器中
        points->InsertNextPoint(x, y, z);
        
        // 将颜色值插入到colors容器中（VTK使用0-255的整数表示颜色）
        unsigned char color[3] = {
            static_cast<unsigned char>(r),
            static_cast<unsigned char>(g),
            static_cast<unsigned char>(b)
        };
        colors->InsertNextTypedTuple(color);
        
        pointCount++;
    }
    
    file.close();
    qDebug() << "成功读取" << pointCount << "个带颜色的点云数据";

    // ------------------------------------------------------------------------
    // 数据管线第2步：包装为VTK的PolyData数据结构
    // ------------------------------------------------------------------------
    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);           // 将点数据绑定到PolyData
    polyData->GetPointData()->SetScalars(colors);  // 将颜色数据绑定到PolyData

    // ------------------------------------------------------------------------
    // 数据管线第3步：数据过滤处理（转换为可渲染的几何体）
    // ------------------------------------------------------------------------
    auto glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
    glyphFilter->SetInputData(polyData);  // 设置输入数据
    glyphFilter->Update();                // 执行过滤操作（必须调用）

    // ------------------------------------------------------------------------
    // 数据管线第4步：返回处理后的数据
    // ------------------------------------------------------------------------
    return glyphFilter->GetOutput();
}

// ============================================================================
// 图形管线函数：将点云数据转换为可视化对象（Actor）
// 流程：vtkPolyData → vtkMapper → vtkActor → 返回Actor
// ============================================================================
vtkSmartPointer<vtkActor> MainWindow::createPointCloudActor(vtkSmartPointer<vtkPolyData> pointCloud)
{
    // ------------------------------------------------------------------------
    // 图形管线第1步：创建映射器（Mapper）
    // ------------------------------------------------------------------------
    // vtkMapper的作用：将几何数据转换为可渲染的图形基元
    // - 输入：几何数据（vtkPolyData）
    // - 输出：图形渲染指令（传递给OpenGL）
    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(pointCloud);  // 连接数据管线的输出
    
    // 关键设置：使用点云数据中的颜色信息
    mapper->SetScalarModeToUsePointData();  // 使用点数据中的标量（颜色）
    mapper->SetColorModeToDirectScalars();  // 直接使用标量作为颜色（不通过查找表映射）
    // 到此：mapper知道了要渲染什么数据以及如何处理颜色

    // ------------------------------------------------------------------------
    // 图形管线第2步：创建Actor（演员/可视化对象）
    // ------------------------------------------------------------------------
    // vtkActor代表场景中的一个可视化对象，包含：
    // - 几何信息（通过Mapper）
    // - 外观属性（颜色、大小、材质等）
    // - 变换矩阵（位置、旋转、缩放）
    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);  // 将Mapper连接到Actor
    // 到此：actor知道了要渲染的数据，并将使用每个点自己的颜色

    // ------------------------------------------------------------------------
    // 图形管线第3步：设置Actor的外观属性
    // ------------------------------------------------------------------------
    // GetProperty()返回vtkProperty对象，用于设置外观
    actor->GetProperty()->SetPointSize(2);              // 点的屏幕尺寸（像素）
    // 注意：不再设置统一的颜色，而是使用每个点自己的颜色
    // actor->GetProperty()->SetColor() - 已移除，改为使用点云数据中的颜色
    
    // 其他可选属性（示例）：
    // actor->GetProperty()->SetOpacity(1.0);           // 不透明度（0.0透明，1.0不透明）
    // actor->GetProperty()->SetAmbient(0.3);           // 环境光系数
    // actor->GetProperty()->SetDiffuse(0.7);           // 漫反射系数
    // actor->GetProperty()->SetSpecular(0.2);          // 镜面反射系数

    // ------------------------------------------------------------------------
    // 图形管线第4步：返回配置好的Actor
    // ------------------------------------------------------------------------
    // 这个Actor可以被添加到vtkRenderer中进行渲染
    return actor;
}
