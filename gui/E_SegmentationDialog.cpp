#include "E_SegmentationDialog.h"
#include "E_Manager.h"
#include <QRect>
#include <QApplication>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkInteractorStyleUser.h>
#include <QVTKOpenGLWidget.h>


E_SegmentationDialog::E_SegmentationDialog(QWidget* parent){
    Initialize();
}

void E_SegmentationDialog::Initialize(){
    
    QVBoxLayout* layout = new QVBoxLayout();
    setLayout(layout);
    
    //Add Widgets
    layout->addWidget(RendererWidgets());
    layout->addWidget(LowerToolbar());


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
    vtkSmartPointer<vtkInteractorStyleUser> interactorStyle = vtkSmartPointer<vtkInteractorStyleUser>::New();
    sliceWidget->SetRenderWindow(sliceRenWin);


    vtkSmartPointer<vtkRenderer> sliceRenderer = E_Manager::SegmentationMgr()->GetSliceRenderer();
    sliceRenderer->GetActiveCamera()->ParallelProjectionOn();
    sliceRenWin->AddRenderer(sliceRenderer);
    sliceRenWin->Render();

    sliceRenWin->GetInteractor()->SetInteractorStyle(interactorStyle);

    return widget;
}

QToolBar* E_SegmentationDialog::LowerToolbar(){
    QRect rec = QApplication::desktop()->screenGeometry();
    int height = rec.height();
    int iconSize = height / 25;

    //Initialize Toolbar
    QToolBar* toolbar = new QToolBar();
    toolbar->setIconSize(QSize(iconSize, iconSize));
    toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->setMovable(false);


    //Play
    QAction* runAction = new QAction(QIcon(":/images/pantone-2.png"), "Run", this);
    connect(runAction, SIGNAL(triggered()), this, SLOT(onStartSegmentation()));
    toolbar->addAction(runAction);

    //Slider
    m_sliceSlider = new QSlider(Qt::Horizontal);
    m_sliceSlider->setSingleStep(1);
    m_sliceSlider->setRange(2, 12);
    connect(m_sliceSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderChange(int)));
    toolbar->addWidget(m_sliceSlider);

    //Save
    QAction* saveAction = new QAction(QIcon(":/images/pantone-2.png"), "Set", this);
    connect(saveAction, SIGNAL(triggered()), this, SLOT(onSaveGroundTruth()));
    toolbar->addAction(saveAction);



    return toolbar;
}

void E_SegmentationDialog::UpdateSlider(int len){
    m_sliceSlider->setRange(2, len-3);
}


/////////////SLOTS/////////////////////////////
void E_SegmentationDialog::onSliderChange(int idx){

    E_Manager::SegmentationMgr()->GetVolume()->SetSlice(E_Volume::SAG, idx);
    E_Manager::SegmentationMgr()->Redraw(false);
}


void E_SegmentationDialog::onStartSegmentation(){
    std::cout << "start segmentation" << std::endl;
}
void E_SegmentationDialog::onFinishSegmentation(){
    std::cout << "segmentation finished" << std::endl;
}
void E_SegmentationDialog::onSaveGroundTruth(){
    std::cout << "Set Ground Truth " << std::endl;
}