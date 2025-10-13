#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vtkSmartPointer.h>

// VTK类前置声明
class vtkRenderer;
class vtkRenderWindow;
class vtkPoints;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkActor;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    // VTK组件
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkRenderWindow> renderWindow;

    // 从文件读取点云数据
    vtkSmartPointer<vtkPolyData> createPointCloudFromFile(const QString& filename);

    // 设置点云 actor
    vtkSmartPointer<vtkActor> createPointCloudActor(vtkSmartPointer<vtkPolyData> pointCloud);
};

#endif // MAINWINDOW_H
