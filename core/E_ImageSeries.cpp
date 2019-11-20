#include "E_ImageSeries.h"
#include <itkThresholdImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>

E_ImageSeries::E_ImageSeries(std::vector<std::string> filenames){
    ///Define Reader
    m_numSlices = filenames.size();
    m_imageContainer = DicomReader::New();
    m_dicomIO = ImageIOType::New();

    m_imageContainer->SetImageIO(m_dicomIO);        
    m_imageContainer->SetFileNames(filenames);
    m_imageContainer->Update();
    m_imageData = m_imageContainer->GetOutput();

    PreprocessImageContainer();

    ///Add To Container
    m_groundTruth = NULL;
}

E_ImageSeries::~E_ImageSeries(){

}

void E_ImageSeries::PreprocessImageContainer(){

        ///Threshold image, minimum -1024;
        itk::ThresholdImageFilter<ImageType>::Pointer clipFilter = itk::ThresholdImageFilter<ImageType>::New();
        clipFilter->SetInput(m_imageData);
        clipFilter->ThresholdBelow(-1000);
        clipFilter->SetOutsideValue(-1000);
        clipFilter->Update();
    
        //Maybe,, have to make new threshold filter here
        // clipFilter->SetInput(clipFilter->GetOutput());
        itk::ThresholdImageFilter<ImageType>::Pointer upperclipFilter = itk::ThresholdImageFilter<ImageType>::New();
        upperclipFilter->SetInput(clipFilter->GetOutput());    
        upperclipFilter->ThresholdAbove(2500);
        upperclipFilter->SetOutsideValue(0);
        upperclipFilter->Update();

        using RescaleIntensityimageFilterType = itk::RescaleIntensityImageFilter<ImageType, ImageType>;
        RescaleIntensityimageFilterType::Pointer normalizer = RescaleIntensityimageFilterType::New();
        normalizer->SetInput(upperclipFilter->GetOutput());
        normalizer->SetOutputMinimum(0);
        normalizer->SetOutputMaximum(255);
        normalizer->Update();



        m_imageData = normalizer->GetOutput();
}

void E_ImageSeries::SetGroundTruth(ImageType::Pointer gtdata){
    m_groundTruth = ImageType::New();
    m_groundTruth = gtdata;
}
std::string E_ImageSeries::GetSeriesDescription(){
    //Show series description with slice numbers
    return GetDicomInfo("0008|103e") + " (" + std::to_string( GetNumberOfSlices() ) + ")";
}

std::string E_ImageSeries::GetDicomInfo(std::string tag){
    using MetaDataStringType = itk::MetaDataObject<std::string>;

    
    const itk::MetaDataDictionary & dictionary =  m_dicomIO->GetMetaDataDictionary();
    auto end = dictionary.End();

    ///Get Entrya ID by tag
    auto entryID = dictionary.Find(tag);

    if(entryID != end){
        itk::MetaDataObjectBase::Pointer tagObject = entryID->second;
        MetaDataStringType::Pointer tagValue = dynamic_cast<MetaDataStringType*>(tagObject.GetPointer());
        
        return tagValue->GetMetaDataObjectValue();

    }else{
        return "unknown";
    }
}

E_ImageSeries::ImageType::Pointer E_ImageSeries::GetImageData(){
    return m_imageData;

}