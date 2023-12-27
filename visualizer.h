#pragma once
#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>
#include <vtkNew.h>
#include <vtkProperty.h>

class Visualizer
{
public:
    Visualizer();
    // void visualizeNode(Octree::Node *node);
    // void visualizeOctree(Octree *octree);
    void render();
    ~Visualizer();

private:
    int figure;
    float size;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkRenderWindow> renderWindow;
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;
    vtkSmartPointer<vtkNamedColors> colors;
};

Visualizer::Visualizer(){
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);
    colors = vtkSmartPointer<vtkNamedColors>::New();
}

// void Visualizer::visualizeNode(Octree::Node *node)
// {
//     if (!node)
//         return;

//     float cubeSize = figure == 1 ? 10 * size : size;

//     vtkSmartPointer<vtkCubeSource> cubeSource = vtkSmartPointer<vtkCubeSource>::New();
//     cubeSource->SetBounds(
//         static_cast<double>(node->point.x), static_cast<double>(node->point.x) + cubeSize,
//         static_cast<double>(node->point.y), static_cast<double>(node->point.y) + cubeSize,
//         static_cast<double>(node->point.z), static_cast<double>(node->point.z) + cubeSize);

//     vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//     mapper->SetInputConnection(cubeSource->GetOutputPort());

//     vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
//     actor->SetMapper(mapper);
//     actor->GetProperty()->SetColor(colors->GetColor3d("Green").GetData());

//     renderer->AddActor(actor);

//     for (int i = 0; i < 8; ++i)
//     {
//         visualizeNode(node->children[i]);
//     }
// }

// void Visualizer::visualizeOctree(Octree *octree)
// {
//     visualizeNode(octree->root);
// }

void Visualizer::render()
{
    renderer->ResetCamera();
    renderer->GetActiveCamera()->Azimuth(30);
    renderer->GetActiveCamera()->Elevation(30);
    renderer->ResetCameraClippingRange();
    renderer->SetBackground(colors->GetColor3d("Silver").GetData());

    renderWindow->SetSize(1200, 800);
    renderWindow->SetWindowName("R*TREE");

    renderWindowInteractor->Initialize();
    renderWindow->Render();
    renderWindowInteractor->Start();
}

Visualizer::~Visualizer()
{
}