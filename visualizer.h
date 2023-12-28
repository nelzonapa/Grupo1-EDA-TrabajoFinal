#pragma once
#include <vtkCubeSource.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>
#include <vtkNew.h>
#include <vtkProperty.h>
#include "rstartree.h"

class Visualizer
{
public:
    Visualizer();
    void drawPoint(float x, float y, float z);
    void drawCube(float x, float y, float z, float w, float h, float d);
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

Visualizer::Visualizer()
{
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);
    colors = vtkSmartPointer<vtkNamedColors>::New();
}
void Visualizer::drawCube(float x, float y, float z, float w, float h, float d)
{
    vtkNew<vtkCubeSource> cube;
    cube->SetCenter(x, y, z);
    cube->SetXLength(w);
    cube->SetYLength(h);
    cube->SetZLength(d);
    cube->SetCenter(x, y, z);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(cube->GetOutputPort());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);

    actor->GetProperty()->SetOpacity(0.2);
    const string colorsArray[] = {"Red", "Green", "Blue", "Yellow", "Cyan", "Magenta"};
    actor->GetProperty()->SetColor(colors->GetColor3d(colorsArray[rand() % 6]).GetData());

    // Add the actor to the scene
    renderer->AddActor(actor);
}
void Visualizer::drawPoint(float x, float y, float z)
{
    vtkNew<vtkSphereSource> sphere;
    sphere->SetRadius(0.1);
    sphere->SetCenter(x, y, z);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(sphere->GetOutputPort());

    vtkNew<vtkActor> actor;
    actor->GetProperty()->SetColor(colors->GetColor3d("Red").GetData());
    actor->SetMapper(mapper);

    // Add the actor to the scene
    renderer->AddActor(actor);
}

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