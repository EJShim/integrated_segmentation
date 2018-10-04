#include "E_SegmentationThread.h"
#include "E_Manager.h"
#include <QCoreApplication>
#include <QDir>
#include <QThread>
#include "itkRegionOfInterestImageFilter.h"

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/cc/ops/array_ops.h"
#include "tensorflow/cc/saved_model/loader.h"



E_SegmentationThread::E_SegmentationThread(){

    Initialize();

    //Initialize Session Container
    std::string path = QCoreApplication::applicationDirPath().toStdString()+"/temp_saved";    
    tensorflow::LoadSavedModel(tensorflow::SessionOptions(), tensorflow::RunOptions(), path, {"ejshim"}, &m_sessionContainer);
}

E_SegmentationThread::~E_SegmentationThread(){
    
}


void E_SegmentationThread::Initialize(){
    m_imageData = NULL;

    m_patientIdx = -1;
    m_sereisIdx = -1;
}

void E_SegmentationThread::SetTargetData(int patientIdx, int seriesIdx){
    m_patientIdx = patientIdx;
    m_sereisIdx = seriesIdx;
}



void E_SegmentationThread::process(){
    
    // if(m_patientIdx == -1 || m_sereisIdx == -1) return;
    //Get Current Image Data
    m_imageData = E_Manager::SegmentationMgr()->GetTargetImage();
    ImageType::SizeType size = m_imageData->GetLargestPossibleRegion().GetSize();
    int slices = int(size[2]);

    //Get Session
    tensorflow::Session* session = m_sessionContainer.session.get();    

    //Tensor minibatch Container
    tensorflow::Tensor inputs(tensorflow::DT_FLOAT, {1,5,512,512});
    
    
    //Test For 10 slices,, in for test in cpu it should be from 2 to slices-2
    for(int i=2; i<slices-2 ; i++){

        //Get Input Slice, Convert to Tensor
        ImageType::Pointer slice = GetSlice(i);

        //This Part, just memory allocation(?) could be better,,
        memcpy(inputs.tensor<float,4>().data(), slice->GetBufferPointer(),inputs.TotalBytes());         
    
        //Forward Inference!
        std::vector<tensorflow::Tensor> outputs;
        session->Run({{ "input_ejshim", inputs }}, {"output_ejshim"}, {}, &outputs);

        //Assign Ground Truth, this part, do memcpy
        AssignGroundTruth(i, outputs[0]);
        
        //Emit Current Progression
        emit onCalculated(i);
    }

    //Make Big Result Tensor,, Emit,, that matters??

    Initialize();
    emit finished();
}

void E_SegmentationThread::SetImageData(ImageType::Pointer data){
    m_imageData = data;   
}

E_SegmentationThread::ImageType::Pointer E_SegmentationThread::GetSlice(int idx){

    ImageType::SizeType inSize = m_imageData->GetLargestPossibleRegion().GetSize();
    if(idx < 2) return nullptr;
    

    ImageType::IndexType start;
    start[0] = 0;
    start[1] = 0;
    start[2] = idx-2;

    //Current, Extract Single Slice
    ImageType::SizeType size;
    size[0] = inSize[0];
    size[1] = inSize[1];
    size[2] = 5;

    ImageType::RegionType desiredRegion;
    desiredRegion.SetSize(size);
    desiredRegion.SetIndex(start);


    typedef itk::RegionOfInterestImageFilter< ImageType, ImageType > ROIFilterType;
    ROIFilterType::Pointer roiFilter = ROIFilterType::New();
    roiFilter->SetRegionOfInterest(desiredRegion);
    roiFilter->SetInput(m_imageData);
    roiFilter->Update();

    return roiFilter->GetOutput();
}

void E_SegmentationThread::AssignGroundTruth(int idx, tensorflow::Tensor tensor){

    //Assign Groudn Truth For Visualization
    vtkSmartPointer<E_Volume> currentVolume = E_Manager::SegmentationMgr()->GetVolume();
    vtkSmartPointer<vtkImageData> vtkImage = currentVolume->GetGroundTruth();
    
    int* dims = vtkImage->GetDimensions();
    int memIdx = dims[0] * dims[1] * idx;
    memcpy(static_cast<float*>(vtkImage->GetScalarPointer())+memIdx, tensor.tensor_data().data(), tensor.TotalBytes());
}