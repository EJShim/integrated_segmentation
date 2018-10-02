#pragma once

#include <QVTKOpenGLWidget.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <itkImage.h>
#include <E_Volume.h>

#include <E_SegmentationDialog.h>

class E_SegmentationManager{

    public:
    typedef itk::Image<float, 3> ImageType;
    E_SegmentationManager();
    ~E_SegmentationManager();


    protected:
    //Segmentation Dialog
    E_SegmentationDialog* m_dialog;

    //3D Renderer
    vtkSmartPointer<vtkRenderer> m_mainRenderer;

    //2D Renderer
    vtkSmartPointer<vtkRenderer> m_sliceRenderer;

    //Volume Actor
    vtkSmartPointer<E_Volume> m_volume;


    public:
    void InitializeSegmentation();

    //Get Dialog
    E_SegmentationDialog* GetDialog();

    //Get Volume
    vtkSmartPointer<E_Volume> GetVolume(){return m_volume;};

    //Redraw
    void Redraw(bool reset = true);

    vtkSmartPointer<vtkRenderer> GetMainRenderer();
    vtkSmartPointer<vtkRenderer> GetSliceRenderer();

};