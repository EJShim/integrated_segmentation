#include "E_Volume.h"
#include <iostream>
#include <vtkImageCast.h>

E_Volume::E_Volume(){
    m_imageData = NULL;

    /// Original Volume
    m_volumeMapper = NULL;
    m_volumeProperty = NULL;
    m_colorFunction = NULL;
    m_opacityFunction = NULL;
    m_imageProperty = NULL;

    //Ground Truth Volume
    m_gt_imageData = NULL;
    m_gt_volume = NULL;
    m_gt_volumeMapper = NULL;
    m_gt_volumeProperty = NULL;
    m_gt_colorFunction = NULL;
    m_gt_opacityFunction = NULL;
    m_gt_imageProperty = NULL;

    /// Ground Truth Image Slice

    for(int i=0 ; i<3 ; i++){
        m_sliceMapper[i] = NULL;
        m_imageSlice[i] = NULL;
        m_gt_sliceMapper[i] = NULL;
        m_gt_sliceMapper3D[i] = NULL;
        m_gt_imageSlice[i] = NULL;
        m_gt_imageSlice3D[i] = NULL;
    }

    m_currentTransferFunctionIdx = -1;
    
}


E_Volume::~E_Volume(){
    
}

void E_Volume::SetImageData(vtkSmartPointer<vtkImageData> imageData){

    m_imageData = imageData;
    
    
    if(m_colorFunction == NULL){
        m_colorFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
        m_opacityFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
        SetTransferFunction(0);
    }else{
        SetTransferFunction(m_currentTransferFunctionIdx);
    }

    if(m_volumeProperty == NULL){
        m_volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
        m_volumeProperty->SetColor(m_colorFunction);
        m_volumeProperty->SetScalarOpacity(m_opacityFunction);
        m_volumeProperty->ShadeOff();
        m_volumeProperty->SetInterpolationTypeToLinear();

        this->SetProperty(m_volumeProperty);
    }    
    
    if(m_volumeMapper == NULL){
        m_volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();        
        m_volumeMapper->SetBlendModeToMaximumIntensity();        
        this->SetMapper(m_volumeMapper);
    }
    
    m_volumeMapper->SetInputData(m_imageData);        
    Update();    
    
    

    //Image
    if(m_imageProperty == NULL){
        m_imageProperty = vtkSmartPointer<vtkImageProperty>::New();
        m_imageProperty->SetInterpolationTypeToLinear();
    }
    double* scalarRange = m_imageData->GetScalarRange();
    double huRange[2] = {-1024, 3096};
    m_imageProperty->SetColorLevel((huRange[1]+huRange[0])/2.0);
    m_imageProperty->SetColorWindow(huRange[1]-huRange[0]-1.0);

    if(m_imageSlice[0] == NULL){
        for(int i=0 ; i<3 ; i++){
            m_sliceMapper[i] = vtkSmartPointer<vtkImageSliceMapper>::New();
            m_sliceMapper[i]->SetOrientation(i);
            // m_sliceMapper[i]->SetInputData(m_imageData);

            m_imageSlice[i] = vtkSmartPointer<vtkImageSlice>::New();
            m_imageSlice[i]->SetProperty(m_imageProperty);
            m_imageSlice[i]->SetMapper(m_sliceMapper[i]);
        }        
    }

    for(int i=0 ; i<3 ; i++){
        m_sliceMapper[i]->SetInputData(m_imageData);
        m_imageSlice[i]->Update();
        int sliceNum = m_sliceMapper[i]->GetSliceNumberMaxValue() / 2;
        m_sliceMapper[i]->SetSliceNumber(sliceNum);
    }
}

void E_Volume::SetGroundTruth(vtkSmartPointer<vtkImageData> imageData){    
    m_gt_imageData = imageData;
    std::cout << m_gt_imageData->GetScalarTypeAsString() << std::endl;

    if(m_gt_volume == NULL){
        m_gt_volume = vtkSmartPointer<vtkVolume>::New();
    }


    if(m_gt_colorFunction == NULL){
        m_gt_colorFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
        m_gt_colorFunction->AddRGBPoint(0, 0.0, 0.0, 1.0);
        m_gt_colorFunction->AddRGBPoint(1, 0.0, 1.0, 0.0);
        m_gt_colorFunction->AddRGBPoint(2, 1.0, 0.0, 0.0);
    }
    

    if(m_gt_opacityFunction == NULL){
        m_gt_opacityFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
        m_gt_opacityFunction->AddPoint(0, 0.0);
        m_gt_opacityFunction->AddPoint(1, 1.0);
        m_gt_opacityFunction->AddPoint(2, 1.0);
    }


    if(m_gt_volumeProperty == NULL){
        m_gt_volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
        m_gt_volumeProperty->SetColor(m_gt_colorFunction);
        m_gt_volumeProperty->SetScalarOpacity(m_gt_opacityFunction);
        m_gt_volumeProperty->ShadeOn();
        m_gt_volumeProperty->SetInterpolationTypeToLinear();

        m_gt_volume->SetProperty(m_gt_volumeProperty);
    }    
    
    if(m_gt_volumeMapper == NULL){
        m_gt_volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();        
        m_gt_volumeMapper->SetBlendModeToComposite();
        m_gt_volume->SetMapper(m_gt_volumeMapper);
    }

    if(m_gt_imageProperty == NULL){
        m_gt_imageProperty = vtkSmartPointer<vtkImageProperty>::New();
        m_gt_imageProperty->SetInterpolationTypeToLinear();                
        m_gt_imageProperty->SetColorLevel(2.0);
        m_gt_imageProperty->SetColorWindow(0.5);
        m_gt_imageProperty->SetOpacity(0.3);
        m_gt_imageProperty->SetLookupTable(m_gt_colorFunction);
    }
    

    if(m_gt_imageSlice[0] == NULL){
        for(int i=0 ; i<3 ; i++){
            m_gt_sliceMapper[i] = vtkSmartPointer<vtkImageSliceMapper>::New();
            m_gt_sliceMapper[i]->SetOrientation(i);            

            m_gt_imageSlice[i] = vtkSmartPointer<vtkImageSlice>::New();
            m_gt_imageSlice[i]->SetProperty(m_gt_imageProperty);
            m_gt_imageSlice[i]->SetMapper(m_gt_sliceMapper[i]);

            //3D Slice
            m_gt_sliceMapper3D[i] = vtkSmartPointer<vtkImageSliceMapper>::New();
            m_gt_sliceMapper3D[i]->SetOrientation(i);            

            m_gt_imageSlice3D[i] = vtkSmartPointer<vtkImageSlice>::New();
            m_gt_imageSlice3D[i]->SetProperty(m_gt_imageProperty);
            m_gt_imageSlice3D[i]->SetMapper(m_gt_sliceMapper3D[i]);
        }
    }
    m_gt_volumeMapper->SetInputData(m_gt_imageData);
    
    m_gt_volume->Update();


    for(int i=0 ; i<3 ; i++){
        m_gt_sliceMapper[i]->SetInputData(m_gt_imageData);
        m_gt_imageSlice[i]->Update();

        m_gt_sliceMapper3D[i]->SetInputData(m_gt_imageData);
        m_gt_imageSlice3D[i]->Update();


        //Set Slice Middle
        int sliceNum = m_gt_sliceMapper[i]->GetSliceNumberMaxValue() / 2;
        m_gt_sliceMapper[i]->SetSliceNumber(sliceNum);
        m_gt_sliceMapper3D[i]->SetSliceNumber(sliceNum);
    }
}


void E_Volume::SetSlice(int idx, int sliceNum){
    if(m_sliceMapper[idx] == NULL) return;
    m_sliceMapper[idx]->SetSliceNumber(sliceNum);

    if(m_gt_sliceMapper[idx] == NULL) return;
    m_gt_sliceMapper[idx]->SetSliceNumber(sliceNum);
    m_gt_sliceMapper3D[idx]->SetSliceNumber(sliceNum);
    
}

void E_Volume::ForwardSlice(int idx){
    
    if(m_sliceMapper[idx] == NULL) return;


    int sliceNum = m_sliceMapper[idx]->GetSliceNumber();
    if(sliceNum >= m_sliceMapper[idx]->GetSliceNumberMaxValue()){
        return;
    }

    SetSlice(idx, sliceNum + 5);
}


void E_Volume::BackwardSlice(int idx){
    if(m_sliceMapper[idx] == NULL) return;

    int sliceNum = m_sliceMapper[idx]->GetSliceNumber();
    if(sliceNum <= m_sliceMapper[idx]->GetSliceNumberMinValue()){
        return;
    }

    SetSlice(idx, sliceNum - 5);
}


void E_Volume::SetTransferFunction(int idx){
    if(m_currentTransferFunctionIdx == idx) return;    


    //Change Status
    m_currentTransferFunctionIdx = idx;
    
    //Remove Current Function Val
    m_colorFunction->RemoveAllPoints();        
    m_opacityFunction->RemoveAllPoints();
    
    

    //Adjust CT Scalar Range
    double* scalarRange = m_imageData->GetScalarRange();
    double huRange[2] = {0, 255};
    
    switch(idx){
        case 0:

        //Color Funciton
        m_colorFunction->AddRGBPoint(scalarRange[0], 1.0, 1.0, 1.0);
        m_colorFunction->AddRGBPoint(huRange[0], 1.0, 1.0, 1.0);
        m_colorFunction->AddRGBPoint(huRange[1], 1.0, 1.0, 1.0);
        m_colorFunction->AddRGBPoint(scalarRange[1], 1.0, 1.0, 1.0);

        //Opacity FUnction
        m_opacityFunction->AddPoint(scalarRange[0], 0.0);
        m_opacityFunction->AddPoint(huRange[0], 0.0);
        m_opacityFunction->AddPoint(huRange[1], 1.0);
        m_opacityFunction->AddPoint(scalarRange[1], 1.0);

        break;

        case 1:
        //Color Function
        m_colorFunction->AddRGBPoint(scalarRange[0], 1.0, 1.0, 1.0);
        // m_colorFunction->AddRGBPoint(scalarRange[1]-1, 1.0, 1.0, 1.0);
        m_colorFunction->AddRGBPoint(scalarRange[1], 0.0, 0.5, 0.0);

        //Opacity FUnction
        m_opacityFunction->AddPoint(scalarRange[0], 0.0);
        m_opacityFunction->AddPoint(scalarRange[1], 1.0);
        break;

        default:
        break;
    }
}