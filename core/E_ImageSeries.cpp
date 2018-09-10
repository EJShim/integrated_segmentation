#include "E_ImageSeries.h"


E_ImageSeries::E_ImageSeries(std::vector<std::string> filenames){
    ///Define Reader
    m_imageContainer = DicomReader::New();
    m_dicomIO = ImageIOType::New();

    m_imageContainer->SetImageIO(m_dicomIO);        
    m_imageContainer->SetFileNames(filenames);
    m_imageContainer->Update();

    ///Add To Container
}

E_ImageSeries::~E_ImageSeries(){

}

std::string E_ImageSeries::GetSeriesDescription(){
    return GetDicomInfo("0008|103e");
}

std::string E_ImageSeries::GetDicomInfo(std::string tag){
    std::cout << "get series description" << std::endl;
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