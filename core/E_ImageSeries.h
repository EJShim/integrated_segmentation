#pragma once
#include <string.h>
#include <vector>

#include <itkImage.h>
#include <itkImageSeriesReader.h>
#include "itkGDCMImageIO.h"


class E_ImageSeries{
    public:
    E_ImageSeries(std::vector<std::string> filenames);
    ~E_ImageSeries();

    
    typedef itk::Image<float, 3> ImageType;
    typedef itk::ImageSeriesReader<ImageType> DicomReader;
    typedef itk::GDCMImageIO ImageIOType;



    protected:
    ///Series UIDs,, it should be merged..
    DicomReader::Pointer m_imageContainer;
    ImageIOType::Pointer m_dicomIO;
    ImageType::Pointer m_groundTruth;


    public:
    DicomReader::Pointer GetImageContainer(){return m_imageContainer;}
    ImageType::Pointer GetGroundTruth(){return m_groundTruth;}
    std::string GetSeriesDescription();

    //Setters
    void SetGroundTruth(ImageType::Pointer gtdata){m_groundTruth = gtdata; }

    ///Dicom Info by its tag
    std::string GetDicomInfo(std::string tag);

};