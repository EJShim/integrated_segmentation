#include "E_Manager.h"

#include <iostream>
#include <vtkCamera.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkProperty2D.h>
#include <vtkContextScene.h>
#include <vtkAxis.h>

#include "E_InteractorStyle.h"


E_Manager::E_Manager(){
    this->Initialize();
}

E_Manager::~E_Manager(){
    this->ClearMemory();
}

E_Manager* E_Manager::m_instance;
E_VolumeManager* E_Manager::m_volumeManager;

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
            this->m_renderer[idx]->GetActiveCamera()->Azimuth(-90.0);
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
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renWin = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

    // Add renderer to renwin
    m_histogramRenderer->SetRenderWindow(renWin);


    // Set Histogram Plot
    m_histogramPlot = vtkSmartPointer<vtkChartXY>::New();
    m_histogramRenderer->GetScene()->AddItem(m_histogramPlot);
    m_histogramPlot->ClearPlots();

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

    // this->m_renderer[idx]->Render();
    this->m_renderer[idx]->GetRenderWindow()->Render();
}

void E_Manager::RedrawAll(bool reset){
    for(int i=0 ; i<NUM_VIEW ; i++){
        this->Redraw(i, reset);
    }

    //Update Histogram
    this->m_histogramRenderer->GetRenderWindow()->Render();
}