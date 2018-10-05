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
    //Rendering Flag
    bool m_bRendering;

    //Segmentation Dialog
    E_SegmentationDialog* m_dialog;

    //3D Renderer
    vtkSmartPointer<vtkRenderer> m_mainRenderer;

    //2D Renderer
    vtkSmartPointer<vtkRenderer> m_sliceRenderer;

    //Volume Actor
    vtkSmartPointer<E_Volume> m_volume;

    /// Target segmentation image and mask
    ImageType::Pointer m_targetImage;
    ImageType::Pointer m_mask;


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

    ImageType::Pointer GetTargetImage(){return m_targetImage;};
    ImageType::Pointer GetMaskImage(){return m_mask;};


    //Start Segmentation
    void OnSegmentationProcess(int idx);
    void FinishSegmentation();


    void OnCloseWork();

    bool IsRendering(){return m_bRendering;}

    protected:
    void UpdateVisualization();
    

};