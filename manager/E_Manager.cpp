#include "E_Manager.h"
#include <QScrollBar>
#include <iostream>
#include <vtkCamera.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkProperty2D.h>
#include <vtkContextScene.h>
#include <vtkAxis.h>
#include "E_InteractorStyle.h"
#include <vtkInteractorStyleUser.h>
#include <vtkContextInteractorStyle.h>

E_Manager::E_Manager(){

    this->Initialize();
}

E_Manager::~E_Manager(){
    this->ClearMemory();
}

E_Manager* E_Manager::m_instance;
E_VolumeManager* E_Manager::m_volumeManager;
E_SegmentationManager* E_Manager::m_segmentationManager;

E_Manager* E_Manager::Mgr(){
    if(m_instance == NULL){
        m_instance = new E_Manager();
        atexit(Destroy);
    }
    return m_instance;
}

E_VolumeManager* E_Manager::VolumeMgr(){
    if(m_volumeManager == NULL){
        m_volumeManager = new E_VolumeManager();
        atexit(DestroyVolumeManager);
    }

    return m_volumeManager;
}

E_SegmentationManager* E_Manager::SegmentationMgr(){
    if(m_segmentationManager == NULL){
        m_segmentationManager = new E_SegmentationManager();
        atexit(DestroySegmentationManager);
    }

    return m_segmentationManager;
}

void E_Manager::Initialize(){
    
}

void E_Manager::ClearMemory(){

}


void E_Manager::SetVTKWidget(QVTKOpenGLWidget* widget, int idx){

    this->m_renderer[idx] = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renWin = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    // renWin->OffScreenRenderingOn();
    renWin->AddRenderer(this->m_renderer[idx]);

    if(idx != 0){
        this->m_renderer[idx]->GetActiveCamera()->ParallelProjectionOn();
    
        //Set 2D Interactor Style
        vtkSmartPointer<E_InteractorStyle> interactorstyle = vtkSmartPointer<E_InteractorStyle>::New();        
        interactorstyle->SetIdx(idx-1);

        vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
        interactor->SetInteractorStyle(interactorstyle);
        renWin->SetInteractor(interactor);

        if(idx == 1){
            this->m_renderer[idx]->GetActiveCamera()->Azimuth(90.0);
        }else if(idx == 2){
            this->m_renderer[idx]->GetActiveCamera()->Elevation(90.0);   
        }
         this->m_renderer[idx]->GetActiveCamera()->OrthogonalizeViewUp();
    }

    widget->SetRenderWindow(renWin);
    renWin->Render();
}

void E_Manager::SetHistogramWidget(QVTKOpenGLWidget* widget){
    //Initialize Renderer and renderwindow
    m_histogramRenderer = vtkSmartPointer<vtkContextView>::New();
    m_histogramRenderer->GetRenderer()->SetBackground(0.0, 0.0, 0.0);

    
    //Initialize Render WIndow and Interactor Style
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renWin = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    
    
    // Add renderer to renwin
    m_histogramRenderer->SetRenderWindow(renWin);

    /// Add Interactor Style??
    vtkSmartPointer<E_ContextInteractorStyle> interactorStyle = vtkSmartPointer<E_ContextInteractorStyle>::New();
    interactorStyle->SetScene(m_histogramRenderer->GetScene());
    m_histogramRenderer->GetRenderWindow()->GetInteractor()->SetInteractorStyle(interactorStyle);


   
    
    m_histogramRenderer->GetScene()->ClearItems();
    


    // Initialize Plot Chart
    m_histogramPlot = vtkSmartPointer<vtkChartXY>::New();
    m_histogramRenderer->GetScene()->AddItem(m_histogramPlot);
    //m_histogramPlot->ClearPlots();

    //Init Histogram Plot
    m_histogramPlot->ForceAxesToBoundsOn();
    m_histogramPlot->SetAutoAxes(false);
    m_histogramPlot->SetAutoSize(true);
    m_histogramPlot->SetHiddenAxisBorder(3);

    for(int i=0 ; i<4 ; i++){
        m_histogramPlot->GetAxis(i)->SetVisible(false);
        m_histogramPlot->GetAxis(i)->SetNumberOfTicks(0);
        m_histogramPlot->GetAxis(i)->SetBehavior(vtkAxis::CUSTOM);
        m_histogramPlot->GetAxis(i)->SetLabelsVisible(false);
        m_histogramPlot->GetAxis(i)->SetTitle("Histogram");
    }

    m_histogramPlot->SetSelectionMethod(vtkChart::SELECTION_ROWS);
    m_histogramPlot->Update();




    // Add Renwin to widget
    widget->SetRenderWindow(renWin);
    renWin->Render();
}

void E_Manager::Redraw(int idx, bool reset){

    if(reset){
        this->m_renderer[idx]->ResetCamera();
        if(idx != 0){
            this->m_renderer[idx]->GetActiveCamera()->Zoom(1.5);
        }
    }

    double* pos = m_renderer[VIEW_SAG]->GetActiveCamera()->GetPosition();
    double* focal = m_renderer[VIEW_SAG]->GetActiveCamera()->GetFocalPoint();

    // this->m_renderer[idx]->Render();
    this->m_renderer[idx]->Render();
    this->m_renderer[idx]->GetRenderWindow()->Render();
}

void E_Manager::RedrawAll(bool reset){
    for(int i=0 ; i<NUM_VIEW ; i++){
        this->Redraw(i, reset);
    }

    //Update Histogram
    this->m_histogramRenderer->GetRenderer()->Render();
    this->m_histogramRenderer->GetRenderWindow()->Render();
    
}

void E_Manager::SetLogWidget(QDockWidget* widgetDocker){
    m_logWidget = new QListWidget();
    m_logWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_logWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    widgetDocker->setWidget(m_logWidget);
}

void E_Manager::PopLog(){
    m_logWidget->takeItem( m_logWidget->count()-1 );

}
void E_Manager::SetLog(const char *arg, ...){
    va_list arguments;

    std::string logstring = "";
    for (va_start(arguments, arg); arg != NULL; arg = va_arg(arguments, const char *)) {    
        logstring.append(arg);
        logstring.append(" ");
    }
    
    m_logWidget->addItem(QString(logstring.c_str()));
    if(m_logWidget->count() > 100){
        m_logWidget->takeItem(0);
    }

    m_logWidget->verticalScrollBar()->setValue( m_logWidget->verticalScrollBar()->maximum());


    va_end(arguments);
}