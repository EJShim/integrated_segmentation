#pragma once

#include <iostream>
#include <string.h>
#include <vector>
#include <itkImage.h>
#include <itkImageSeriesReader.h>
#include "itkGDCMImageIO.h"

#include <E_ImageSeries.h>


class E_DicomSeries{    
    

    public:
    E_DicomSeries();
    ~E_DicomSeries();

    
    typedef itk::Image<float, 3> ImageType;
    typedef itk::ImageSeriesReader<ImageType> DicomReader;
    typedef itk::GDCMImageIO ImageIOType;

    protected:
    ///Study Description
    std::string m_studyDescription;

    ///Series UIDs,, it should be merged..
    std::vector<E_ImageSeries*> m_imageSeries;    


    public:

    ///Set Directory Path
    void SetPath(const char* path);

    ///Set Ground Truth
    void SetGroundTruth(ImageType::Pointer gt, int idx){m_imageSeries[idx]->SetGroundTruth(gt);}
    ImageType::Pointer GetGroundTruth(int idx){return m_imageSeries[idx]->GetGroundTruth();}
    bool IsGroundTruthExist(int idx);

    ///Get Number of Child Serieses
    int GetNumberOfSerieses(){return m_imageSeries.size();}

    /// Get Study Description in string
    std::string GetStudyDescription(){return m_studyDescription;}

    ///Get Image Container
    // std::vector<DicomReader::Pointer> &GetImageContainer(){return m_imageContainer;}
    DicomReader::Pointer GetImageContainer(int idx){return m_imageSeries[idx]->GetImageContainer();}

    /// Get Series Description in STring
    // std::vector<std::string> &GetSeriesDescription(){return m_seriesDescription;}
    std::string GetSeriesDescription(int idx){return m_imageSeries[idx]->GetSeriesDescription();}


};