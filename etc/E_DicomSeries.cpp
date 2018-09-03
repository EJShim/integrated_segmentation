#include "E_DicomSeries.h"
#include "itkGDCMSeriesFileNames.h"

E_DicomSeries::E_DicomSeries(){
    m_studyDescription = "unknown";
}


E_DicomSeries::~E_DicomSeries(){
    m_imageContainer.clear();
    m_seriesDescription.clear();
}

void E_DicomSeries::SetPath(const char* path){

    std::cout << path << std::endl;

    typedef itk::GDCMSeriesFileNames NamesGeneratorType;    
    NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
    nameGenerator->SetUseSeriesDetails(true);
    nameGenerator->AddSeriesRestriction("0008|0021");
    nameGenerator->SetDirectory(path);

    ///Series UIDs
    m_seriesUIDs = nameGenerator->GetSeriesUIDs();

    //Use Vector Iterator
    std::vector<std::string>::const_iterator seriesItr = m_seriesUIDs.begin();
    std::vector<std::string>::const_iterator seriesEnd = m_seriesUIDs.end();


    m_studyDescription = "";
    while (seriesItr != seriesEnd)
    {
        std::vector<std::string> fileNames = nameGenerator->GetFileNames(seriesItr->c_str());
        // if(fileNames.size() <= 1) continue;

        ///Define Reader
        DicomReader::Pointer reader = DicomReader::New();
        ImageIOType::Pointer dicomIO = ImageIOType::New();
        reader->SetImageIO(dicomIO);        
        reader->SetFileNames(fileNames);
        reader->Update();

        ///Add To Container
        m_imageContainer.push_back(reader);

        ///Get Study Description
        if(m_studyDescription.length() == 0){
            m_studyDescription = GetDicomTag(dicomIO, "0008|1030");
        }
        ///Add Study Description
        std::string seriesDescription = GetDicomTag(dicomIO, "0008|103e") + " (" + std::to_string(fileNames.size()) + ")" ;
        m_seriesDescription.push_back(seriesDescription);

        ++seriesItr;
    }
}


std::string E_DicomSeries::GetDicomTag(ImageIOType::Pointer dicomIO, std::string tag){
    using MetaDataStringType = itk::MetaDataObject<std::string>;

    
    const itk::MetaDataDictionary & dictionary =  dicomIO->GetMetaDataDictionary();
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