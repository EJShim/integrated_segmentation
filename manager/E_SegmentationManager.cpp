#include "E_SegmentationManager.h"
#include "E_Manager.h"
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkCamera.h>
#include <E_Volume.h>

E_SegmentationManager::E_SegmentationManager(){
    m_dialog = NULL;
    m_volume = nullptr;

    m_mainRenderer = NULL;
    m_sliceRenderer = NULL;
}

E_SegmentationManager::~E_SegmentationManager(){

}

void E_SegmentationManager::InitializeSegmentation(){

    ImageType::Pointer itkImage = E_Manager::VolumeMgr()->GetCurrentImageData();
    if(itkImage == nullptr){
        E_Manager::Mgr()->SetLog("selecte image first", NULL);

        return;
    }

    vtkSmartPointer<vtkImageData> imageData = E_Manager::VolumeMgr()->ConvertITKtoVTKImageData(itkImage, false);
    int* dims = imageData->GetDimensions();
    GetDialog()->UpdateSlider(dims[2]);

    //Make Volume
    if(m_volume == nullptr){
        m_volume = vtkSmartPointer<E_Volume>::New();
    }else{
        GetMainRenderer()->RemoveVolume(m_volume);
        GetSliceRenderer()->RemoveViewProp(m_volume->GetImageSlice(E_Volume::SAG));
    }


    m_volume->SetImageData(imageData);
    m_volume->Update();

    GetMainRenderer()->AddVolume(m_volume);
    GetSliceRenderer()->AddViewProp(m_volume->GetImageSlice(E_Volume::SAG));
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
    //For Renderwindow Initialization
    if(m_dialog == NULL) m_dialog = new E_SegmentationDialog();

    if(reset){
        GetSliceRenderer()->ResetCamera();
        GetMainRenderer()->ResetCamera();
    }


    GetMainRenderer()->GetRenderWindow()->Render();
    GetSliceRenderer()->GetRenderWindow()->Render();
}