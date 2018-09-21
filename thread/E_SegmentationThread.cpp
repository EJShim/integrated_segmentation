#include "E_SegmentationThread.h"
#include <QCoreApplication>
#include <QDir>
#include <QThread>
#include "itkRegionOfInterestImageFilter.h"


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
    if(m_imageData == nullptr) return;
    qRegisterMetaType<tensorflow::Tensor>("tensorflow::Tensor");


    //temp - use resource in the future
    std::string path = QCoreApplication::applicationDirPath().toStdString()+"/temp_saved";
    tensorflow::SavedModelBundle bundle;
    //why should I do tensorflow:: here??

    
    tensorflow::LoadSavedModel(tensorflow::SessionOptions(), tensorflow::RunOptions(), path, {"ejshim"}, &bundle);
    tensorflow::Session* session = bundle.session.get();    
    ImageType::SizeType size = m_imageData->GetLargestPossibleRegion().GetSize();
    int slices = int(size[2]);    


    //Tensor minibatch Container
    tensorflow::Tensor inputs(tensorflow::DT_FLOAT, {1,5,512,512});
    
    
    //Test For 10 slices,, in for test in cpu it should be from 2 to slices-2
    for(int i=2; i<slices-2 ; i++){

        //Get Input Slice, Convert to Tensor
        ImageType::Pointer slice = GetSlice(i);

        //std::copy_n(slice->GetBufferPointer(), inputs.TotalBytes(), inputs.tensor<float,4>().data());
        memcpy(inputs.tensor<float,4>().data(), slice->GetBufferPointer(),inputs.TotalBytes());         

        std::vector<tensorflow::Tensor> outputs;
        //Forward Inference!
        session->Run({{ "input_ejshim", inputs }}, {"output_ejshim"}, {}, &outputs);

        std::cout << i << std::endl;
        emit onCalculated(i, outputs[0]);
    }

    //Make Big Result Tensor,, Emit,, that matters??
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


// tensorflow::Tensor E_SegmentationThread::ConvertImageToTensor(vtkSmartPointer<vtkImageData> image){

//     int* dims = image->GetDimensions();  
//     int* extents = image->GetExtent();

//     // std::cout << extents[0] << ',' << extents[1] << ',' << extents[2] << ',' << extents[3] << ',' << extents[4] << ',' << extents[5] << std::endl;

//     tensorflow::Tensor imageTensor(tensorflow::DT_FLOAT, {1, dims[0], dims[1], dims[2]});
//     auto tensorMapper = imageTensor.tensor<float,4>();

//     for(int z=0 ; z<dims[2] ; z++){
//         for(int y=0 ; y<dims[1] ; y++){
//             for(int x=0 ; x<dims[0] ; x++){
//                 float* pointer = static_cast<float*>(image->GetScalarPointer(x+extents[0], y+extents[2], z+extents[4]));        
//                 tensorMapper(0, x, y, z) = pointer[0];
//             }
//         }
//     }

//     return imageTensor;
// }