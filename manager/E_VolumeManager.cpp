#include "E_Manager.h"
#include "E_VolumeManager.h"

#include <vtkLookupTable.h>
#include <vtkExtractVOI.h>
#include <vtkImageData.h>

#include <vtkImageMagnitude.h>
#include <vtkImageAccumulate.h>
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
#include <vtkPlot.h>
#include <vtkChartXY.h>
#include <vtkAxis.h>

#include <itkNiftiImageIO.h>
#include "tensorflow/core/framework/tensor.h"

#include "itkGDCMSeriesFileNames.h"



E_VolumeManager::E_VolumeManager(){        
    this->m_volume = NULL;

    m_bVolumeInRenderer = false;
    m_bGTInRenderer = false;
    
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

void E_VolumeManager::UpdateHistogram(){

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

    
    // chart->AddPlot(opacityFunctionItem);
    // chart->AddPlot(opacityFunctionPlot);
    chart->AddPlot(colorFunctionItem);
    chart->AddPlot(opacityController);
    


    double scalarRange[2];
    imageData->GetScalarRange(scalarRange);
    int minHU = scalarRange[0];
    int maxHU = scalarRange[1];

    // vtkSmartPointer<vtkImageAccumulate> histogram = vtkSmartPointer<vtkImageAccumulate>::New();
    // histogram->SetInputData(imageData);
    // histogram->SetComponentExtent(0, maxHU-minHU-1, 0, 0, 0, 0);
    // histogram->SetComponentOrigin(minHU, 0, 0);
    // histogram->SetComponentSpacing(1, 0, 0);
    // histogram->Update();

    // //add histogram to table
    // vtkSmartPointer<vtkTable> table = vtkSmartPointer<vtkTable>::New();
    // vtkSmartPointer<vtkIntArray> intArr = vtkSmartPointer<vtkIntArray>::New();
    // vtkSmartPointer<vtkFloatArray> floatArr = vtkSmartPointer<vtkFloatArray>::New();

    // intArr->SetName("scalar");
    // floatArr->SetName("histogram");
    // table->AddColumn(intArr);
    // table->AddColumn(floatArr);

    // int numRow = (int)(maxHU-minHU);
    // table->SetNumberOfRows(numRow);

    // for(int i=0 ; i<numRow; i++){
    //     float val = histogram->GetOutput()->GetScalarComponentAsFloat(i, 0, 0, 0);
    //     table->SetValue(i, 0, i + minHU);
    //     table->SetValue(i, 1, val + 1);
    // }

    // vtkSmartPointer<vtkPlot> line;
    // line = chart->AddPlot( vtkChart::STACKED);
    // line->SetInputData(table, 0, 1);
    // line->SetColor(255, 0, 0, 255);
    // line->Update();

    

    // double hisRange[2];
    // histogram->GetOutput()->GetScalarRange(hisRange);

    chart->GetAxis(vtkAxis::BOTTOM)->SetRange(minHU, maxHU);
    chart->GetAxis(vtkAxis::BOTTOM)->Update();
    // chart->GetAxis(vtkAxis::LEFT)->SetRange(1, hisRange[1]+1);
    chart->GetAxis(vtkAxis::LEFT)->SetRange(1, maxHU+1);
    chart->GetAxis(vtkAxis::LEFT)->LogScaleOn();
    chart->GetAxis(vtkAxis::LEFT)->Update();

}


void E_VolumeManager::AddVolume(ImageType::Pointer itkImageData){

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
    }

    UpdateHistogram();
    E_Manager::Mgr()->RedrawAll(true);
}

void E_VolumeManager::AddSelectedVolume(int patientIdx, int seriesIdx){
    if(m_patientList.size() < patientIdx + 1 ){
        return;
    }

    if(m_patientList[patientIdx]->GetNumberOfSerieses() < seriesIdx+1){
        return;
    }

    ///Get Image Container from dicom group
    DicomReader::Pointer container = m_patientList[patientIdx]->GetImageContainer()[seriesIdx];

    //Add To Renderer
    AddVolume(container->GetOutput());
}