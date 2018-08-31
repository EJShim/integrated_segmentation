#include "E_SegmentationThread.h"

#include <vtkExtractVOI.h>

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/cc/ops/array_ops.h"
#include "tensorflow/cc/saved_model/loader.h"



E_SegmentationThread::E_SegmentationThread(){

    m_imageData = NULL;
}

E_SegmentationThread::~E_SegmentationThread(){
    
}



void E_SegmentationThread::process(){
    if(m_imageData == NULL) return;

    qRegisterMetaType<tensorflow::Tensor>("tensorflow::Tensor");

    //temp - use resource in the future
    std::string path = "D:/projects/weights";
    tensorflow::SavedModelBundle bundle;
    //why should I do tensorflow:: here??
    tensorflow::LoadSavedModel(tensorflow::SessionOptions(), tensorflow::RunOptions(), path, {"serve"}, &bundle);
    tensorflow::Session* session = bundle.session.get();
    
    for(int z=2; z<m_dims[2]-2 ; z++){
        vtkSmartPointer<vtkImageData> slice = GetSingleBatchImage(z);
        tensorflow::Tensor singleBatch = ConvertImageToTensor(slice);

        // Run the session, evaluating our "c" operation from the graph  
        std::vector<tensorflow::Tensor> preprocessed_outputs;
        std::vector<tensorflow::Tensor> outputs;
        session->Run({{ "ej_input", singleBatch }}, {"EJ_PREPROCESSED"}, {}, &preprocessed_outputs);
        session->Run({{ "input_1", preprocessed_outputs[0] }}, {"EJ_OUTPUT"}, {}, &outputs);

        // std::cout << "processing " << z << " out of " << m_dims[2]-3  << std::endl;

        //Emit Signal
        emit onCalculated(z, outputs[0]);
    }

    emit finished();
}

void E_SegmentationThread::SetImageData(vtkSmartPointer<vtkImageData> data){
    m_imageData = vtkSmartPointer<vtkImageData>::New();
    m_imageData->DeepCopy(data);
    m_dims = m_imageData->GetDimensions();   
}



vtkSmartPointer<vtkImageData> E_SegmentationThread::GetSingleBatchImage(int slice){

    vtkSmartPointer<vtkExtractVOI> batchExtractor = vtkSmartPointer<vtkExtractVOI>::New();
    batchExtractor->SetInputData(m_imageData);

    batchExtractor->SetVOI(32, m_dims[0]-32-1, 32, m_dims[1]-32-1, slice-2, slice+2);
    batchExtractor->Update();    

    return batchExtractor->GetOutput();   
}

tensorflow::Tensor E_SegmentationThread::ConvertImageToTensor(vtkSmartPointer<vtkImageData> image){

    int* dims = image->GetDimensions();  
    int* extents = image->GetExtent();

    // std::cout << extents[0] << ',' << extents[1] << ',' << extents[2] << ',' << extents[3] << ',' << extents[4] << ',' << extents[5] << std::endl;

    tensorflow::Tensor imageTensor(tensorflow::DT_FLOAT, {1, dims[0], dims[1], dims[2]});
    auto tensorMapper = imageTensor.tensor<float,4>();

    for(int z=0 ; z<dims[2] ; z++){
        for(int y=0 ; y<dims[1] ; y++){
            for(int x=0 ; x<dims[0] ; x++){
                float* pointer = static_cast<float*>(image->GetScalarPointer(x+extents[0], y+extents[2], z+extents[4]));        
                tensorMapper(0, x, y, z) = pointer[0];
            }
        }
    }

    return imageTensor;
}