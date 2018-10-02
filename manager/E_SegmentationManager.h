#pragma once

#include <QVTKOpenGLWidget.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>


#include <E_SegmentationDialog.h>

class E_SegmentationManager{
    public:
    E_SegmentationManager();
    ~E_SegmentationManager();


    protected:
    //Segmentation Dialog
    E_SegmentationDialog* m_dialog;

    //3D Renderer
    vtkSmartPointer<vtkRenderer> m_mainRenderer;

    //2D Renderer
    vtkSmartPointer<vtkRenderer> m_sliceRenderer;



    public:
    void InitializeSegmentation();

    E_SegmentationDialog* GetDialog();

    vtkSmartPointer<vtkRenderer> GetMainRenderer();
    vtkSmartPointer<vtkRenderer> GetSliceRenderer();

};