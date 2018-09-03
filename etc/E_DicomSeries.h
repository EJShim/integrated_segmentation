#pragma once

#include <iostream>
#include <string.h>
#include <vector>


#include <itkImage.h>
#include <itkImageSeriesReader.h>
#include "itkGDCMImageIO.h"


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

    ///Series UIDs
    std::vector<DicomReader::Pointer> m_imageContainer;
    std::vector<std::string> m_seriesUIDs;
    std::vector<std::string> m_seriesDescription;


    public:

    ///Set Directory Path
    void SetPath(const char* path);

    ///Get Number of Child Serieses
    int GetNumberOfSerieses(){return m_seriesUIDs.size();}

    /// Get Study Description in string
    std::string GetStudyDescription(){return m_studyDescription;}

    ///Get Image Container
    std::vector<DicomReader::Pointer> &GetImageContainer(){return m_imageContainer;}

    /// Get Series Description in STring
    std::vector<std::string> &GetSeriesDescriptions(){return m_seriesDescription;}

    protected:
    std::string GetDicomTag(ImageIOType::Pointer dicomIO, std::string tag);

};