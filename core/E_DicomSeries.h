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
    // std::vector<DicomReader::Pointer> m_imageContainer;
    // std::vector<ImageType::Pointer> m_groundTruth;
    // std::vector<std::string> m_seriesUIDs;
    // std::vector<std::string> m_seriesDescription;


    public:

    ///Set Directory Path
    void SetPath(const char* path);

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

    protected:
    // std::string GetDicomTag(ImageIOType::Pointer dicomIO, std::string tag);

};