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
    std::string path = "D:/projects/integrated_segmentation/build/bin/temp_saved";
    tensorflow::SavedModelBundle bundle;
    //why should I do tensorflow:: here??
    tensorflow::LoadSavedModel(tensorflow::SessionOptions(), tensorflow::RunOptions(), path, {"ejshim"}, &bundle);
    tensorflow::Session* session = bundle.session.get();
    
    
    //Iterate Over y-axis, it is axial image
    for(int y=2; y<m_dims[1]-2 ; y++){
        vtkSmartPointer<vtkImageData> slice = GetSingleBatchImage(y);
        
        int* dims = slice->GetDimensions();
        tensorflow::Tensor singleBatch(tensorflow::DT_FLOAT, {1, dims[0], dims[1], dims[2]});
        memcpy( singleBatch.tensor<float,4>().data(), slice->GetScalarPointer(), singleBatch.TotalBytes());
        
        // // Run the session, evaluating our "c" operation from the graph          
        std::vector<tensorflow::Tensor> outputs;
        session->Run({{ "input_ejshim", singleBatch }}, {"output_ejshim"}, {}, &outputs);

        //Emit Signal
        emit onCalculated(y, outputs[0]);
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

    batchExtractor->SetVOI(0, m_dims[0]-1, slice-2, slice+2, 0, m_dims[2]-1);
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