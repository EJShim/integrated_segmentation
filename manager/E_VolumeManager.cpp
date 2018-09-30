#include "E_Manager.h"
#include "E_VolumeManager.h"
#include <string.h>

#include <vtkLookupTable.h>
#include <vtkExtractVOI.h>
#include <vtkImageData.h>

#include <vtkImageMagnitude.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageAccumulate.h>
#include <vtkIntArray.h>
#include <vtkFieldData.h>
#include <vtkBarChartActor.h>
#include<vtkLegendBoxActor.h>
#include <vtkColorTransferFunctionItem.h>
#include <vtkPiecewiseFunctionItem.h>
#include <vtkPiecewiseControlPointsItem.h>
#include <vtkFloatArray.h>
#include <vtkTable.h>
#include <vtkChartXY.h>
#include <vtkAxis.h>
#include <itkImageDuplicator.h>
#include <itkNiftiImageIO.h>
#include "tensorflow/core/framework/tensor.h"
#include <itkThresholdImageFilter.h>
#include "itkBinaryThresholdImageFilter.h"
#include "itkGDCMSeriesFileNames.h"
#include <math.h>



E_VolumeManager::E_VolumeManager(){        
    this->m_volume = NULL;

    m_bVolumeInRenderer = false;
    m_bGTInRenderer = false;

    m_histogramGraph = NULL;

    m_currentSelectedParentIdx = -1;
    m_currentSelectedSeries = -1;
    
}

E_VolumeManager::~E_VolumeManager(){

}
void E_VolumeManager::ImportDicom(const char* path){

    E_Manager::Mgr()->SetLog("import from ", path, NULL);
    E_DicomSeries* series = new E_DicomSeries();
    series->SetPath(path);
    m_patientList.push_back(series);

}

void E_VolumeManager::ImportGroundTruth(const char* path, int parentIdx, int childIdx){

    E_DicomSeries* series = new E_DicomSeries();
    series->SetPath(path);

    if(series->GetNumberOfSerieses() > 1){
        E_Manager::Mgr()->SetLog("Something wrong, attempt to import file that has more than 2 serieses", NULL);
        return;
    }

    //Get ImageData
    DicomReader::Pointer container = series->GetImageContainer(0);


    ImageCalculatorFilterType::Pointer imageCalculatorFilter = ImageCalculatorFilterType::New ();
    imageCalculatorFilter->SetImage(container->GetOutput());
    imageCalculatorFilter->Compute();

    ///Threshold image, minimum -1024;
    itk::BinaryThresholdImageFilter<ImageType, ImageType>::Pointer thresholdfilter = itk::BinaryThresholdImageFilter<ImageType, ImageType>::New();
    thresholdfilter->SetInput(container->GetOutput());
    thresholdfilter->SetLowerThreshold(imageCalculatorFilter->GetMaximum());
    thresholdfilter->SetUpperThreshold(imageCalculatorFilter->GetMaximum());
    thresholdfilter->SetInsideValue(1);
    thresholdfilter->SetOutsideValue(0);
    thresholdfilter->Update();

    // Set Ground Truth
    m_patientList[parentIdx]->SetGroundTruth(thresholdfilter->GetOutput(), childIdx);
    E_Manager::Mgr()->SetLog("Ground Truth Set", NULL);
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
    if(m_volume == NULL|| m_currentSelectedParentIdx==-1 || m_currentSelectedSeries == -1) return;

    ////////////Here, Use ITK
    ImageType::Pointer itkImage = m_patientList[m_currentSelectedParentIdx]->GetImageData(m_currentSelectedSeries);
        
    //Deep Copy Current Image
    typedef itk::ImageDuplicator<ImageType> DuplicatorType;
    DuplicatorType::Pointer duplicator = DuplicatorType::New();
    duplicator->SetInputImage(itkImage);
    duplicator->Update();
    ImageType::Pointer gtImage = duplicator->GetOutput();

    //Make Zero Tensor
    ImageType::SizeType size = gtImage->GetLargestPossibleRegion().GetSize();
    tensorflow::Tensor zero_tensor(tensorflow::DT_FLOAT, {int(size[0]), int(size[1]), int(size[2])});
    memcpy(gtImage->GetBufferPointer(), zero_tensor.tensor_data().data(), zero_tensor.TotalBytes());

    //Set Ground Truth
    m_patientList[m_currentSelectedParentIdx]->SetGroundTruth(gtImage, m_currentSelectedSeries);

    //Add To Renderer
    AddGroundTruth(m_currentSelectedParentIdx, m_currentSelectedSeries);
    
    E_Manager::Mgr()->RedrawAll(false);    
}

void E_VolumeManager::UpdateGroundTruth(int idx){

    //Get Current Ground Truth Image.
    ImageType::Pointer itkImage = GetCurrentGroundTruthData();
    ImageType::SizeType size = itkImage->GetLargestPossibleRegion().GetSize();

    

    //Get Current VTK ImageData
    vtkSmartPointer<vtkImageData> vtkImage = m_volume->GetGroundTruth();
    // vtkImage->AllocateScalars(VTK_FLOAT,1);
    // int* dims = vtkImage->GetDimensions();

    // //Fill vtk imagedata
    // for (int z = 0; z < dims[2]; z++){
    //     for (int y = idx; y < idx+1; y++){
    //         for (int x = 0; x < dims[0]; x++){
    //         float* pixel = static_cast<float*>(vtkImage->GetScalarPointer(x,y,z));
    //         pixel[0] = 1.0;
    //         }
    //     }
    // }

    
    //Memcopy Tensor information to the gt
    
    int memoryIdx = int(size[0]) * int(size[1]) * idx;
    memcpy(static_cast<float*>(vtkImage->GetScalarPointer(0,idx,0)), itkImage->GetBufferPointer()+memoryIdx, 512*512*sizeof(float));

    

    m_volume->Update();
    // std::cout << size <<  "|| " << dims[0] << "," << dims[1] << "," << dims[2] << std::endl;
    

}


void E_VolumeManager::InitializeHistogram(){
 // Get Plot Data
    vtkSmartPointer<vtkImageData> imageData = m_volume->GetImageData();
    vtkSmartPointer<vtkColorTransferFunction> colorFunction = m_volume->GetColorTransferFunction();
    vtkSmartPointer<vtkPiecewiseFunction> opacityFunction = m_volume->GetOpacityTransferFunction();

    ///Opacity FUnction Plot and contoller
    vtkSmartPointer<vtkPiecewiseControlPointsItem> opacityController = vtkSmartPointer<vtkPiecewiseControlPointsItem>::New();
    opacityController->SetPiecewiseFunction(opacityFunction);
    vtkSmartPointer<vtkPiecewiseFunctionItem> opacityFunctionPlot = vtkSmartPointer<vtkPiecewiseFunctionItem>::New();
    opacityFunctionPlot->SetPiecewiseFunction(opacityFunction);

    /// Color Function Plot
    vtkSmartPointer<vtkColorTransferFunctionItem> colorFunctionItem = vtkSmartPointer<vtkColorTransferFunctionItem>::New();
    colorFunctionItem->SetColorTransferFunction(colorFunction);
    
    
    // Get Plot Renderer and Clear
    vtkSmartPointer<vtkChartXY> chart = E_Manager::Mgr()->GetHistogramPlot();
    chart->ClearPlots();
    chart->AddPlot(colorFunctionItem);
    chart->AddPlot(opacityController);
    // chart->AddPlot(opacityFunctionItem);
    // chart->AddPlot(opacityFunctionPlot);
    
    
    chart->GetAxis(vtkAxis::LEFT)->SetRange(0, 1);
    chart->GetAxis(vtkAxis::LEFT)->Update();


    return;
    //Histogram
    m_histogramGraph = chart->AddPlot( vtkChart::STACKED);
    m_histogramGraph->SetColor(255, 0, 0, 255);
    m_histogramGraph->SelectableOff();
    m_histogramGraph->Update();
}

void E_VolumeManager::UpdateHistogram(){
    //Get Image Data and chart
    vtkSmartPointer<vtkImageData> imageData = m_volume->GetImageData();
    vtkSmartPointer<vtkChartXY> chart = E_Manager::Mgr()->GetHistogramPlot();

    //Update X-axis Scale
    double scalarRange[2];
    imageData->GetScalarRange(scalarRange);
    int minHU = scalarRange[0];
    int maxHU = scalarRange[1];

    chart->GetAxis(vtkAxis::BOTTOM)->SetRange(minHU, maxHU);
    chart->GetAxis(vtkAxis::BOTTOM)->Update();
    E_Manager::Mgr()->SetLog("Scalar Range", std::to_string(minHU).c_str(), std::to_string(maxHU).c_str(), NULL);


    if(m_histogramGraph == NULL)
        return;
    //Update Histogram
    vtkSmartPointer<vtkImageAccumulate> histogram = vtkSmartPointer<vtkImageAccumulate>::New();
    histogram->SetInputData(imageData);
    histogram->SetComponentExtent(0, maxHU-minHU-1, 0, 0, 0, 0);
    histogram->SetComponentOrigin(minHU, 0, 0);
    histogram->SetComponentSpacing(10, 0, 0);
    histogram->SetIgnoreZero( false );
    histogram->Update();

    double hisRange[2];
    histogram->GetOutput()->GetScalarRange(hisRange);
    int minHIS = int(hisRange[0]);
    int maxHIS = int(hisRange[1]);
    E_Manager::Mgr()->SetLog("Histogram Range : ", std::to_string(minHIS).c_str(), std::to_string(maxHIS).c_str(), NULL);

    //add histogram to table
    vtkSmartPointer<vtkTable> table = vtkSmartPointer<vtkTable>::New();
    vtkSmartPointer<vtkIntArray> intArr = vtkSmartPointer<vtkIntArray>::New();
    vtkSmartPointer<vtkFloatArray> floatArr = vtkSmartPointer<vtkFloatArray>::New();
    intArr->SetName("scalar");
    floatArr->SetName("histogram");
    int numRow = (int)(maxHU-minHU);
    table->SetNumberOfRows(numRow);
    for(int i=0 ; i<numRow; i++){
        float val = histogram->GetOutput()->GetScalarComponentAsFloat(i, 0, 0, 0);
        intArr->InsertNextValue(i);
        floatArr->InsertNextValue( log(val)/log(maxHIS));
    }
    table->AddColumn(intArr);
    table->AddColumn(floatArr);

    //Update Plot
    m_histogramGraph->SetInputData(table, 0, 1);
}


void E_VolumeManager::AddVolume(vtkSmartPointer<vtkImageData> vtkImageData){

      //Make Volume
    if(m_volume == NULL){
        m_volume = vtkSmartPointer<E_Volume>::New();        
    }    
    m_volume->SetImageData(vtkImageData);    

    if(!m_bVolumeInRenderer){
        E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_MAIN)->AddViewProp(m_volume);
        E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_AXL)->AddViewProp(m_volume->GetImageSlice(0));
        E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_COR)->AddViewProp(m_volume->GetImageSlice(1));
        E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_SAG)->AddViewProp(m_volume->GetImageSlice(2));

        //Add Histogram Items
        InitializeHistogram();
        
        m_bVolumeInRenderer = true;
    }

    UpdateHistogram();
}

void E_VolumeManager::AddVolume(ImageType::Pointer itkImageData){
    RemoveGroundTruth();


    vtkSmartPointer<vtkImageData> imageData = ConvertITKtoVTKImageData(itkImageData);    

    AddVolume(imageData);

}

void E_VolumeManager::AddSelectedVolume(int patientIdx, int seriesIdx){
    if(m_patientList.size() < patientIdx + 1 ){
        return;
    }
    if(m_patientList[patientIdx]->GetNumberOfSerieses() < seriesIdx+1){
        return;
    }

    ///Get Image Container from dicom group
    // DicomReader::Pointer container = m_patientList[patientIdx]->GetImageContainer(seriesIdx);
    ImageType::Pointer itkImageData = m_patientList[patientIdx]->GetImageData(seriesIdx);
    
    //Add To Renderer
    AddVolume(itkImageData);


    ///Change Current OTF according to series description
    std::string description = m_patientList[patientIdx]->GetStudyDescription(); 
    E_Manager::Mgr()->SetLog(description.c_str(), NULL);
    
    if(description.find("segmentation") != std::string::npos){
        E_Manager::Mgr()->SetLog( "This is Segmentation Volume" , NULL);
        m_comboBox->setCurrentIndex(1);
    }else{
        m_comboBox->setCurrentIndex(0);
    }

        //Update Current Selected Volume
    m_currentSelectedParentIdx = patientIdx;
    m_currentSelectedSeries = seriesIdx;

    E_Manager::Mgr()->RedrawAll(true);
}


void E_VolumeManager::AddGroundTruth(int parentIdx, int childIdx){
    if(m_volume == NULL) return;


    ///Set Ground Truth
    ImageType::Pointer itkImage = m_patientList[parentIdx]->GetGroundTruth(childIdx);
    vtkSmartPointer<vtkImageData> imageData = ConvertITKtoVTKImageData(itkImage);
    

    //Update GT Image to the volume
    m_volume->SetGroundTruth(imageData);


    // Add To Renderer
    if(!m_bGTInRenderer){
        E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_MAIN)->AddViewProp(m_volume->GetGroundTruthVolume());
        E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_AXL)->AddViewProp(m_volume->GetGroundTruthImageSlice(0));
        E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_COR)->AddViewProp(m_volume->GetGroundTruthImageSlice(1));
        E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_SAG)->AddViewProp(m_volume->GetGroundTruthImageSlice(2));
    
        m_bGTInRenderer = true;
    }
    E_Manager::Mgr()->RedrawAll(false);

}

void E_VolumeManager::RemoveGroundTruth(){
    if(!m_bGTInRenderer) return;
    
    //Remove Ground Truth Volume And Slice
    E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_MAIN)->RemoveViewProp(m_volume->GetGroundTruthVolume());
    E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_AXL)->RemoveViewProp(m_volume->GetGroundTruthImageSlice(0));
    E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_COR)->RemoveViewProp(m_volume->GetGroundTruthImageSlice(1));
    E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_SAG)->RemoveViewProp(m_volume->GetGroundTruthImageSlice(2));
    E_Manager::Mgr()->RedrawAll(false);

    m_bGTInRenderer = false;
}

vtkSmartPointer<vtkImageData> E_VolumeManager::ConvertITKtoVTKImageData(ImageType::Pointer itkImage){
    OrientImageFilterType::Pointer orienter = OrientImageFilterType::New();
    orienter->UseImageDirectionOn();
    orienter->SetInput(itkImage);
    orienter->Update();
    
    // Convert to vtkimagedataclear
    itkVtkConverter::Pointer conv = itkVtkConverter::New();
    conv->SetInput(orienter->GetOutput());
    conv->Update();

    //should deep copy, or the pointer will be removed here.
    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    imageData->DeepCopy(conv->GetOutput());

    return imageData;

}

E_VolumeManager::ImageType::Pointer E_VolumeManager::GetCurrentImageData(){
    if(m_currentSelectedParentIdx == -1 || m_currentSelectedSeries == -1) return NULL;

    return m_patientList[m_currentSelectedParentIdx]->GetImageData(m_currentSelectedSeries);
}

E_VolumeManager::ImageType::Pointer E_VolumeManager::GetCurrentGroundTruthData(){
    if(m_currentSelectedParentIdx == -1 || m_currentSelectedSeries == -1) return NULL;

    return m_patientList[m_currentSelectedParentIdx]->GetGroundTruth(m_currentSelectedSeries);
}