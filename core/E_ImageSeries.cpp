#include "E_ImageSeries.h"
#include <itkThresholdImageFilter.h>

E_ImageSeries::E_ImageSeries(std::vector<std::string> filenames){
    ///Define Reader
    m_numSlices = filenames.size();
    m_imageContainer = DicomReader::New();
    m_dicomIO = ImageIOType::New();

    m_imageContainer->SetImageIO(m_dicomIO);        
    m_imageContainer->SetFileNames(filenames);
    m_imageContainer->Update();

    ///Add To Container
    m_groundTruth = NULL;
}

E_ImageSeries::~E_ImageSeries(){

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
        ///Threshold image, minimum -1024;
    itk::ThresholdImageFilter<ImageType>::Pointer clipFilter = itk::ThresholdImageFilter<ImageType>::New();
    clipFilter->SetInput(GetImageContainer()->GetOutput());
    clipFilter->ThresholdBelow(-1024);
    clipFilter->SetOutsideValue(-1024);
    clipFilter->Update();

    //Maybe,, have to make new threshold filter here
    // clipFilter->SetInput(clipFilter->GetOutput());
    itk::ThresholdImageFilter<ImageType>::Pointer upperclipFilter = itk::ThresholdImageFilter<ImageType>::New();
    upperclipFilter->SetInput(clipFilter->GetOutput());
    upperclipFilter->ThresholdAbove(3096);
    upperclipFilter->SetOutsideValue(0);
    upperclipFilter->Update();


    return upperclipFilter->GetOutput();

}