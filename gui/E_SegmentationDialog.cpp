#include "E_SegmentationDialog.h"
#include "E_Manager.h"
#include <QRect>
#include <QApplication>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <QVTKOpenGLWidget.h>

E_SegmentationDialog::E_SegmentationDialog(QWidget* parent){
    Initialize();
}

void E_SegmentationDialog::Initialize(){
    
    QVBoxLayout* layout = new QVBoxLayout();
    setLayout(layout);
    
    //Add Widgets
    layout->addWidget(RendererWidgets(), 5);
    layout->addWidget(ButtonWidgets(), 1);




    //Resize according to the screen size
    QRect rec = QApplication::desktop()->screenGeometry();
    int height = rec.height();
    int width = rec.width();
    resize(width*0.75, height*0.75);
    
    
}

QWidget* E_SegmentationDialog::RendererWidgets(){
    QWidget* widget = new QWidget();

    widget->setLayout(new QHBoxLayout());

    //Set Main Widget
    QVTKOpenGLWidget* mainWidget = new QVTKOpenGLWidget();
    widget->layout()->addWidget(mainWidget);
    
    //Renwin Pipelinse
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renWin = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    mainWidget->SetRenderWindow(renWin);

    //Get Renderer, add to renwin
    vtkSmartPointer<vtkRenderer> renderer = E_Manager::SegmentationMgr()->GetMainRenderer();
    renWin->AddRenderer(renderer);
    renWin->Render();


    //Set Slice Widget
    QVTKOpenGLWidget* sliceWidget = new QVTKOpenGLWidget();
    widget->layout()->addWidget(sliceWidget);

    vtkSmartPointer<vtkGenericOpenGLRenderWindow> sliceRenWin = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    sliceWidget->SetRenderWindow(sliceRenWin);

    vtkSmartPointer<vtkRenderer> sliceRenderer = E_Manager::SegmentationMgr()->GetSliceRenderer();
    sliceRenWin->AddRenderer(sliceRenderer);
    sliceRenWin->Render();

    return widget;
}

QWidget* E_SegmentationDialog::ButtonWidgets(){
    QWidget* widget = new QWidget();
    widget->setLayout(new QHBoxLayout());

    return widget;
}