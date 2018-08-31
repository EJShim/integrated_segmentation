#include "E_Manager.h"
#include "E_VolumeManager.h"

#include <vtkLookupTable.h>
#include <vtkExtractVOI.h>
#include <vtkImageData.h>

#include <itkNiftiImageIO.h>
#include "tensorflow/core/framework/tensor.h"

#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"



E_VolumeManager::E_VolumeManager(){        
    this->m_volume = NULL;

    m_bVolumeInRenderer = false;
    m_bGTInRenderer = false;
    
}

E_VolumeManager::~E_VolumeManager(){

}
void E_VolumeManager::ImportDicom(const char* path){
    std::cout << "Import Dicoms from " << path << std::endl;

    

    typedef itk::GDCMImageIO ImageIOType;

    typedef itk::GDCMSeriesFileNames NamesGeneratorType;    
    NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
    nameGenerator->SetUseSeriesDetails(true);
    nameGenerator->AddSeriesRestriction("0008|0021");
    nameGenerator->SetDirectory(path);

    typedef std::vector<std::string> SeriesIdContainer;
    const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();
    SeriesIdContainer::const_iterator seriesItr = seriesUID.begin();
    SeriesIdContainer::const_iterator seriesEnd = seriesUID.end();


    std::vector<DicomReader::Pointer> imageDataList; 

    while (seriesItr != seriesEnd)
    {
        std::vector<std::string> fileNames = nameGenerator->GetFileNames(seriesItr->c_str());

        std::cout << fileNames.size() << std::endl;

        ///Define Reader
        DicomReader::Pointer reader = DicomReader::New();
        ImageIOType::Pointer dicomIO = ImageIOType::New();
        reader->SetImageIO(dicomIO);        
        reader->SetFileNames(fileNames);
        reader->Update();

        imageDataList.push_back(reader);

        ++seriesItr;
    }

    std::cout << "Number of Sereises : " << imageDataList.size() << std::endl;


    ///////////////////Test Volume, from here, refactoring needed! ///////////////////////////////////////////
    ImageType::Pointer itkImageData = imageDataList[1]->GetOutput();


    ///Add Orientation
    OrientImageFilterType::Pointer orienter = OrientImageFilterType::New();
    orienter->UseImageDirectionOn();
    orienter->SetInput(itkImageData);
    orienter->Update();

    // Convert to vtkimagedataclear
    itkVtkConverter::Pointer conv = itkVtkConverter::New();
    conv->SetInput(orienter->GetOutput());
    conv->Update();

    double scalarRange[2];
    conv->GetOutput()->GetScalarRange(scalarRange);
    std::cout << "scalar Range : " << scalarRange[0] << "," << scalarRange[1] << std::endl;

      //Make Volume
    if(m_volume == NULL){
        m_volume = vtkSmartPointer<E_Volume>::New();        
    }
    m_volume->SetImageData(conv->GetOutput());
    

    if(!m_bVolumeInRenderer){
        E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_MAIN)->AddViewProp(m_volume);
        for(int i=0 ; i<NUMSLICE ; i++){
            vtkSmartPointer<vtkImageSlice> slice = m_volume->GetImageSlice(i);
            E_Manager::Mgr()->GetRenderer(i+1)->AddViewProp(slice);
        }
        
        m_bVolumeInRenderer = true;
    }
    else{
        UpdateVolume(m_volume);
        // E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_MAIN)->RemoveViewProp(m_volume);
        // E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_MAIN)->AddViewProp(m_volume);        
    }

    E_Manager::Mgr()->RedrawAll(true);

}



void E_VolumeManager::ImportNII(const char* path){
    // Make ITK Image Data    
    NIIReader::Pointer reader = NIIReader::New();
    reader->SetFileName(path);
    reader->Update();
    ImageType::Pointer itkImageData = reader->GetOutput();

        ///Add Orientation
    OrientImageFilterType::Pointer orienter = OrientImageFilterType::New();
    orienter->UseImageDirectionOn();
    orienter->SetInput(itkImageData);
    orienter->Update();
    
    // Convert to vtkimagedataclear
    itkVtkConverter::Pointer conv = itkVtkConverter::New();
    conv->SetInput(orienter->GetOutput());
    conv->Update();

    //Make Volume
    if(m_volume == NULL){
        m_volume = vtkSmartPointer<E_Volume>::New();        
    }
    m_volume->SetImageData(conv->GetOutput());
    

    if(!m_bVolumeInRenderer){
        E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_MAIN)->AddViewProp(m_volume);
        for(int i=0 ; i<NUMSLICE ; i++){
            vtkSmartPointer<vtkImageSlice> slice = m_volume->GetImageSlice(i);
            E_Manager::Mgr()->GetRenderer(i+1)->AddViewProp(slice);
        }
        
        m_bVolumeInRenderer = true;
    }
    else{
        UpdateVolume(m_volume);
        // E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_MAIN)->RemoveViewProp(m_volume);
        // E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_MAIN)->AddViewProp(m_volume);        
    }

    E_Manager::Mgr()->RedrawAll(true);
}

void E_VolumeManager::ImportGroundTruth(std::string path){
    if(m_volume == NULL) return;

    // Make ITK Image Data    
    NIIReader::Pointer reader = NIIReader::New();
    reader->SetFileName(path);
    reader->Update();
    ImageType::Pointer itkImageData = reader->GetOutput();

            ///Add Orientation
    OrientImageFilterType::Pointer orienter = OrientImageFilterType::New();
    orienter->UseImageDirectionOn();
    orienter->SetInput(itkImageData);
    orienter->Update();
    

    // Convert to vtkimagedata    
    itkVtkConverter::Pointer conv = itkVtkConverter::New();
    conv->SetInput(orienter->GetOutput());
    conv->Update();
    
    //Update GT Image to the volume
    m_volume->SetGroundTruth(conv->GetOutput());

    // Add To Renderer

    if(!m_bGTInRenderer){
        E_Manager::Mgr()->GetRenderer(0)->AddViewProp(m_volume->GetGroundTruthVolume());
        for(int i=0 ; i<3 ; i++){
            vtkSmartPointer<vtkImageSlice> slice = m_volume->GetGroundTruthImageSlice(i);
            //E_Manager::Mgr()->GetRenderer(0)->AddViewProp(slice);
            E_Manager::Mgr()->GetRenderer(i+1)->AddViewProp(slice);
        }
        m_bGTInRenderer = true;
    }else{
        UpdateVolume(m_volume->GetGroundTruthVolume());
    }
    
    

    E_Manager::Mgr()->RedrawAll(true);    
}

void E_VolumeManager::ForwardSlice(int idx){
    if(m_volume == NULL) return;

    m_volume->ForwardSlice(idx);

    E_Manager::Mgr()->Redraw(0);
    E_Manager::Mgr()->Redraw(idx+1);
}

void E_VolumeManager::BackwardSlice(int idx){
    if(m_volume == NULL) return;

    m_volume->BackwardSlice(idx);
    
    E_Manager::Mgr()->Redraw(0);
    E_Manager::Mgr()->Redraw(idx+1);
}

void E_VolumeManager::Toggle3DSlice(int idx, int state){
    if(m_volume == NULL) return;
    if(m_volume->GetGroundTruthVolume() == NULL) return;
    

    vtkSmartPointer<vtkImageSlice> slice = m_volume->GetGroundTruthImageSlice(idx);
    if(state == 0){ ///Show        
        E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_MAIN)->RemoveViewProp(slice);
    }else if(state == 2){//hide
        E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_MAIN)->AddViewProp(slice);
    }else{
        return;
    }

    E_Manager::Mgr()->Redraw(0);
    E_Manager::Mgr()->Redraw(idx+1);

    return;
}

void E_VolumeManager::MakeBlankGroundTruth(){
    if(m_volume == NULL) return;    

    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    imageData->DeepCopy(m_volume->GetImageData());

    int* dims = imageData->GetDimensions();

    vtkSmartPointer<vtkExtractVOI> extractVOI = vtkSmartPointer<vtkExtractVOI>::New();
    extractVOI->SetInputData(imageData);
    extractVOI->SetVOI(32, dims[0]-32-1, 32, dims[1]-32-1, 0, dims[2]);
    extractVOI->Update();

    vtkSmartPointer<vtkImageData> croppedVolume = extractVOI->GetOutput();
    dims = croppedVolume->GetDimensions();

    //Set volume to zero
    tensorflow::Tensor zero_tensor(tensorflow::DT_FLOAT, { dims[0], dims[1], dims[2]});
    int* pointer = static_cast<int*>(croppedVolume->GetScalarPointer());
    std::copy_n(zero_tensor.tensor<float,3>().data(), zero_tensor.tensor<float,3>().size(), pointer);

    m_volume->SetGroundTruth(croppedVolume);
    
    //Show Ground Truth Volume
    E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_MAIN)->AddViewProp(m_volume->GetGroundTruthVolume());
    for(int i=0 ; i<3 ; i++){
        vtkSmartPointer<vtkImageSlice> slice = m_volume->GetGroundTruthImageSlice(i);
        E_Manager::Mgr()->GetRenderer(0)->AddViewProp(slice);
        E_Manager::Mgr()->GetRenderer(i+1)->AddViewProp(slice);
    }
    E_Manager::Mgr()->RedrawAll(false);    
}

void E_VolumeManager::UpdateVolume(vtkSmartPointer<vtkVolume> volume){
    E_Manager::Mgr()->GetRenderer(0)->RemoveViewProp(volume);
    E_Manager::Mgr()->GetRenderer(0)->AddViewProp(volume);
}
