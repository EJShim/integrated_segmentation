#include "E_DicomSeries.h"
#include "itkGDCMSeriesFileNames.h"

E_DicomSeries::E_DicomSeries(){
    m_studyDescription = "unknown";
}


E_DicomSeries::~E_DicomSeries(){
    m_imageSeries.clear();
}

void E_DicomSeries::SetPath(const char* path){

    typedef itk::GDCMSeriesFileNames NamesGeneratorType;    
    NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
    nameGenerator->SetUseSeriesDetails(true);
    nameGenerator->AddSeriesRestriction("0008|0021");
    nameGenerator->SetDirectory(path);

    ///Series UIDs
    std::vector<std::string> seriesUIDs = nameGenerator->GetSeriesUIDs();

    //Use Vector Iterator
    std::vector<std::string>::const_iterator seriesItr = seriesUIDs.begin();
    std::vector<std::string>::const_iterator seriesEnd = seriesUIDs.end();

    char* copy = strdup(path);
    char *token, *last;
    last = token = strtok(copy, "/");
    for (;(token = strtok(NULL, "/")) != NULL; last = token);
    free(copy);

    m_studyDescription = last;
    int descLength = m_studyDescription.length();
    while (seriesItr != seriesEnd)
    {
        std::vector<std::string> fileNames = nameGenerator->GetFileNames(seriesItr->c_str());

        //If FileNames.size == 1, ignore
        if(fileNames.size() == 1){
            ++seriesItr;
            continue;
        }
        // if(fileNames.size() <= 1) continue;

        E_ImageSeries* imageSeries = new E_ImageSeries(fileNames);


        ///Get Study Description
        if(m_studyDescription.length() == descLength){
            m_studyDescription.append("(");
            m_studyDescription.append( imageSeries->GetDicomInfo("0008|1030") );
            m_studyDescription.append(")");
        }

        m_imageSeries.push_back(imageSeries);

        ++seriesItr;
    }
}

bool E_DicomSeries::IsGroundTruthExist(int idx){
    if(m_imageSeries[idx]->GetGroundTruth() == nullptr){
        return false;
    }else{
        return true;
    }
}


// std::string E_DicomSeries::GetDicomTag(ImageIOType::Pointer dicomIO, std::string tag){
//     using MetaDataStringType = itk::MetaDataObject<std::string>;

    
//     const itk::MetaDataDictionary & dictionary =  dicomIO->GetMetaDataDictionary();
//     auto end = dictionary.End();

//     ///Get Entrya ID by tag
//     auto entryID = dictionary.Find(tag);

//     if(entryID != end){
//         itk::MetaDataObjectBase::Pointer tagObject = entryID->second;
//         MetaDataStringType::Pointer tagValue = dynamic_cast<MetaDataStringType*>(tagObject.GetPointer());
        
//         return tagValue->GetMetaDataObjectValue();

//     }else{
//         return "unknown";
//     }
// }