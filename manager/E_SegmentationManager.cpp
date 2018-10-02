#include "E_SegmentationManager.h"
#include "E_Manager.h"

E_SegmentationManager::E_SegmentationManager(){
    m_dialog = NULL;

    m_mainRenderer = NULL;
    m_sliceRenderer = NULL;
}

E_SegmentationManager::~E_SegmentationManager(){

}

void E_SegmentationManager::InitializeSegmentation(){
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