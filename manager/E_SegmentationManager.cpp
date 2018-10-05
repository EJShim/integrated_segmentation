#include "E_SegmentationManager.h"
#include "E_Manager.h"
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkCamera.h>
#include <vtkPointData.h>
#include <E_Volume.h>

E_SegmentationManager::E_SegmentationManager(){
    m_dialog = NULL;
    m_volume = nullptr;

    m_mainRenderer = NULL;
    m_sliceRenderer = NULL;

    m_targetImage = nullptr;
    m_mask = nullptr;

    m_bRendering = false;
}

E_SegmentationManager::~E_SegmentationManager(){

}

void E_SegmentationManager::InitializeSegmentation(){

    m_targetImage = E_Manager::VolumeMgr()->GetCurrentImageData();
    if(m_targetImage == nullptr){
        E_Manager::Mgr()->SetLog("selecte image first", NULL);

        return;
    }

    vtkSmartPointer<vtkImageData> imageData = E_Manager::VolumeMgr()->ConvertITKtoVTKImageData(m_targetImage, false);
    int* dims = imageData->GetDimensions();
    GetDialog()->UpdateSlider(dims[2]);

    //Make Volume
    if(m_volume == nullptr){
        m_volume = vtkSmartPointer<E_Volume>::New();
    }


    m_volume->SetImageData(imageData);
    m_volume->Update();

    GetMainRenderer()->AddViewProp(m_volume);
    GetSliceRenderer()->AddViewProp(m_volume->GetImageSlice(E_Volume::SAG));
    
    
    //Make Blandk Ground Truth and set
    m_mask = E_Manager::VolumeMgr()->MakeBlankGroundTruth(m_targetImage);

    vtkSmartPointer<vtkImageData> maskData = E_Manager::VolumeMgr()->ConvertITKtoVTKImageData(m_mask, false);
    m_volume->SetGroundTruth(maskData);
    m_volume->SetSlice(E_Volume::SAG, GetDialog()->GetSliderValue());

    GetMainRenderer()->AddViewProp(m_volume->GetGroundTruthVolume());
    GetMainRenderer()->AddViewProp(m_volume->GetGroundTruthImageSlice3D(E_Volume::SAG));
    GetSliceRenderer()->AddViewProp(m_volume->GetGroundTruthImageSlice(E_Volume::SAG));
    
    
    Redraw();

    GetDialog()->exec();

}


E_SegmentationDialog* E_SegmentationManager::GetDialog(){
    if(m_dialog == NULL) m_dialog = new E_SegmentationDialog();

    return m_dialog;
}

vtkSmartPointer<vtkRenderer> E_SegmentationManager::GetMainRenderer(){
    if(m_mainRenderer == NULL){
        m_mainRenderer = vtkSmartPointer<vtkRenderer>::New();
    }

    return m_mainRenderer;
}


vtkSmartPointer<vtkRenderer> E_SegmentationManager::GetSliceRenderer(){
    if(m_sliceRenderer == NULL){
        m_sliceRenderer = vtkSmartPointer<vtkRenderer>::New();
    }

    return m_sliceRenderer;
}

void E_SegmentationManager::Redraw(bool reset){

    m_bRendering = true;
    //For Renderwindow Initialization
    if(m_dialog == NULL) m_dialog = new E_SegmentationDialog();

    if(reset){
        GetSliceRenderer()->ResetCamera();
        GetMainRenderer()->ResetCamera();
    }


    GetMainRenderer()->GetRenderWindow()->Render();
    GetSliceRenderer()->GetRenderWindow()->Render();

    m_bRendering = false;
}


void E_SegmentationManager::OnSegmentationProcess(int idx){
    // UpdateVisualization();
}

void E_SegmentationManager::FinishSegmentation(){
    UpdateVisualization();
    Redraw(false);
}

void E_SegmentationManager::OnCloseWork(){
    //Remove Volumes From Renderer
    GetMainRenderer()->RemoveViewProp(m_volume);
    GetSliceRenderer()->RemoveViewProp(m_volume->GetImageSlice(E_Volume::SAG));


    //Remove Ground Truth if it is in
     if(m_mask != nullptr){
         GetMainRenderer()->RemoveViewProp(m_volume->GetGroundTruthVolume());
         GetMainRenderer()->RemoveViewProp(m_volume->GetGroundTruthImageSlice3D(E_Volume::SAG));
         GetSliceRenderer()->RemoveViewProp(m_volume->GetGroundTruthImageSlice(E_Volume::SAG));
     }

     m_mask = nullptr;
}

void E_SegmentationManager::UpdateVisualization(){
    
    if(m_mask == nullptr) return;
    //Try memcpy way
    vtkSmartPointer<vtkImageData> maskData = m_volume->GetGroundTruth();    

    //Assign
    int* dims = maskData->GetDimensions();
    memcpy(m_mask->GetBufferPointer(), maskData->GetScalarPointer(), dims[0]*dims[1]*dims[2]*sizeof(float));

    //Update Visualization
    maskData->GetPointData()->GetScalars()->Modified();
}