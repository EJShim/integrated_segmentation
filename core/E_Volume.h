#ifndef E_VOLUME_H
#define E_VOLUME_H
#include <vtkSmartPointer.h>
#include <vtkVolume.h>
#include <vtkImageData.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkImageProperty.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkLookupTable.h>

#include "tensorflow/core/framework/tensor.h"

class E_Volume : public vtkVolume{
    public:
    vtkTypeMacro(E_Volume, vtkVolume);
    static E_Volume *New() {return new E_Volume;}

    public:
    E_Volume();
    ~E_Volume();

    protected:

    vtkSmartPointer<vtkImageData> m_imageData;

    /// Original Volume
    vtkSmartPointer<vtkSmartVolumeMapper> m_volumeMapper;
    vtkSmartPointer<vtkVolumeProperty> m_volumeProperty;
    vtkSmartPointer<vtkColorTransferFunction> m_colorFunction;
    vtkSmartPointer<vtkPiecewiseFunction> m_opacityFunction;

    /// Original Image Slice
    vtkSmartPointer<vtkImageProperty> m_imageProperty;
    vtkSmartPointer<vtkImageSliceMapper> m_sliceMapper[3];
    vtkSmartPointer<vtkImageSlice> m_imageSlice[3];

    //Ground Truth Volume
    vtkSmartPointer<vtkImageData> m_gt_imageData;
    vtkSmartPointer<vtkVolume> m_gt_volume;
    vtkSmartPointer<vtkSmartVolumeMapper> m_gt_volumeMapper;
    vtkSmartPointer<vtkVolumeProperty> m_gt_volumeProperty;
    vtkSmartPointer<vtkColorTransferFunction> m_gt_colorFunction;
    vtkSmartPointer<vtkPiecewiseFunction> m_gt_opacityFunction;
    vtkSmartPointer<vtkLookupTable> m_gt_lut;

    /// Ground Truth Image Slice
    vtkSmartPointer<vtkImageProperty> m_gt_imageProperty;
    vtkSmartPointer<vtkImageSliceMapper> m_gt_sliceMapper[3];
    vtkSmartPointer<vtkImageSlice> m_gt_imageSlice[3];



    public:    
    void SetImageData(vtkSmartPointer<vtkImageData>);
    void SetGroundTruth(vtkSmartPointer<vtkImageData>);
    void AssignGroundTruthVolume(int slice, tensorflow::Tensor tensorImage);

    vtkSmartPointer<vtkImageData> GetImageData(){return m_imageData;}
    vtkSmartPointer<vtkImageSlice> GetImageSlice(int idx){return m_imageSlice[idx];}
    vtkSmartPointer<vtkVolume> GetGroundTruthVolume(){return m_gt_volume;}
    vtkSmartPointer<vtkImageSlice> GetGroundTruthImageSlice(int idx){return m_gt_imageSlice[idx];}

    void SetSlice(int idx, int sliceNum);
    void ForwardSlice(int idx);
    void BackwardSlice(int idx);

};

#endif //E_VOLUME_H