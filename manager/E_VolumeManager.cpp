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
    
}

E_VolumeManager::~E_VolumeManager(){

}
void E_VolumeManager::ImportDicom(const char* path){

    E_Manager::Mgr()->SetLog("import from ", path, NULL);
    E_DicomSeries* series = new E_DicomSeries();
    series->SetPath(path);
    m_patientList.push_back(series);

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
        // m_comboBox->setEnabled(true);
    }
    m_volume->SetImageData(conv->GetOutput());
    

    if(!m_bVolumeInRenderer){
        E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_MAIN)->AddViewProp(m_volume);
        E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_AXL)->AddViewProp(m_volume->GetImageSlice(0));
        E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_COR)->AddViewProp(m_volume->GetImageSlice(1));
        E_Manager::Mgr()->GetRenderer(E_Manager::VIEW_SAG)->AddViewProp(m_volume->GetImageSlice(2));

        m_bVolumeInRenderer = true;
    }

    E_Manager::Mgr()->RedrawAll(true);
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

           ///Add Orientation
    OrientImageFilterType::Pointer orienter = OrientImageFilterType::New();
    orienter->UseImageDirectionOn();
    orienter->SetInput(container->GetOutput());
    orienter->Update();


    ImageCalculatorFilterType::Pointer imageCalculatorFilter = ImageCalculatorFilterType::New ();
    imageCalculatorFilter->SetImage(orienter->GetOutput());
    imageCalculatorFilter->Compute();

    ///Threshold image, minimum -1024;
    itk::BinaryThresholdImageFilter<ImageType, ImageType>::Pointer thresholdfilter = itk::BinaryThresholdImageFilter<ImageType, ImageType>::New();
    thresholdfilter->SetInput(orienter->GetOutput());
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

       ///Add Orientation
    OrientImageFilterType::Pointer orienter = OrientImageFilterType::New();
    orienter->UseImageDirectionOn();
    orienter->SetInput(itkImageData);
    orienter->Update();

    ///Threshold image, minimum -1024;
    itk::ThresholdImageFilter<ImageType>::Pointer clipFilter = itk::ThresholdImageFilter<ImageType>::New();
    clipFilter->SetInput(orienter->GetOutput());
    clipFilter->ThresholdOutside(-1024, 3096);
    clipFilter->Update();

    // Convert to vtkimagedataclear
    itkVtkConverter::Pointer conv = itkVtkConverter::New();
    conv->SetInput(clipFilter->GetOutput());
    conv->Update();


    AddVolume(conv->GetOutput());

}

void E_VolumeManager::AddSelectedVolume(int patientIdx, int seriesIdx){
    if(m_patientList.size() < patientIdx + 1 ){
        return;
    }
    if(m_patientList[patientIdx]->GetNumberOfSerieses() < seriesIdx+1){
        return;
    }

    ///Get Image Container from dicom group
    DicomReader::Pointer container = m_patientList[patientIdx]->GetImageContainer(seriesIdx);

    //Add To Renderer
    AddVolume(container->GetOutput());


    ///Change Current OTF according to series description
    std::string description = m_patientList[patientIdx]->GetStudyDescription(); 
    E_Manager::Mgr()->SetLog(description.c_str(), NULL);
    
    if(description.find("segmentation") != std::string::npos){
        E_Manager::Mgr()->SetLog( "This is Segmentation Volume" , NULL);
        m_comboBox->setCurrentIndex(1);
    }else{
        m_comboBox->setCurrentIndex(0);
    }

    E_Manager::Mgr()->RedrawAll(true);
}


void E_VolumeManager::AddGroundTruth(int parentIdx, int childIdx){
    if(m_volume == NULL) return;

    ///Set Ground Truth
    ImageType::Pointer itkImage = m_patientList[parentIdx]->GetGroundTruth(childIdx);

    // Convert to vtkimagedata    
    itkVtkConverter::Pointer conv = itkVtkConverter::New();
    conv->SetInput(itkImage);
    conv->Update();
    
    //Update GT Image to the volume
    m_volume->SetGroundTruth(conv->GetOutput());

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