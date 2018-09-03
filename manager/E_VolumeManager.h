#ifndef E_VOLUMEMANAGER_H
#define E_VOLUMEMANAGER_H

#include <iostream>

#include <vtkSmartPointer.h>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageSeriesReader.h>
#include <itkImageToVTKImageFilter.h>
#include <itkOrientImageFilter.h>
#include "itkGDCMImageIO.h"
#include <E_DicomSeries.h>

#include "E_Volume.h"

class E_VolumeManager{
    public:
    enum{AXL, COR, SAG};
    static const int NUMSLICE = 3;

    ///ITK Image Type
    typedef itk::Image<float, 3> ImageType;
    typedef itk::ImageFileReader<ImageType> NIIReader;
    typedef itk::ImageSeriesReader<ImageType> DicomReader;
    typedef itk::ImageToVTKImageFilter<ImageType> itkVtkConverter;
    typedef itk::OrientImageFilter<ImageType, ImageType> OrientImageFilterType;
    typedef itk::GDCMImageIO ImageIOType;



    public:
    E_VolumeManager();
    ~E_VolumeManager();

    protected:   
    bool m_bVolumeInRenderer;
    bool m_bGTInRenderer;
    
    vtkSmartPointer<E_Volume> m_volume;


    ///Dicom Series
    std::vector<E_DicomSeries*> m_patientList;

    public:
    //Import Data
    void ImportNII(const char* path);
    void ImportDicom(const char* path);
    void ImportGroundTruth(std::string path);

    //move slice
    void ForwardSlice(int idx);
    void BackwardSlice(int idx);

    //Show and hide 3d slice
    void Toggle3DSlice(int idx, int state);

    //Make Empty image data
    void MakeBlankGroundTruth();

    /// Because of bug
    void UpdateVolume(vtkSmartPointer<vtkVolume> volume);
    void UpdateHistogram();

    ///Add Selected Volume
    void AddVolume(ImageType::Pointer itkImageData);
    void AddSelectedVolume(int patientIdx, int seriesIdx);

    public:

    vtkSmartPointer<E_Volume> GetCurrentVolume(){return m_volume;}

    ///Get Volume List
    std::vector<E_DicomSeries*> &GetVolumeList(){return m_patientList;}


};

#endif //E_VOLUMEMANAGER_H