#include "E_Volume.h"
#include <iostream>

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
    m_gt_lut = NULL;

    /// Ground Truth Image Slice

    for(int i=0 ; i<3 ; i++){
        m_sliceMapper[i] = NULL;
        m_imageSlice[i] = NULL;
        m_gt_sliceMapper[i] = NULL;
        m_gt_imageSlice[i] = NULL;
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
    m_volumeMapper->Update();    
    
    

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
            m_sliceMapper[i]->SetInputData(m_imageData);

            m_imageSlice[i] = vtkSmartPointer<vtkImageSlice>::New();
            m_imageSlice[i]->SetProperty(m_imageProperty);
            m_imageSlice[i]->SetMapper(m_sliceMapper[i]);
        }        
    }

    for(int i=0 ; i<3 ; i++){
        int sliceNum = m_sliceMapper[i]->GetSliceNumberMaxValue() / 2;
        m_sliceMapper[i]->SetSliceNumber(sliceNum);
    }    
}

void E_Volume::SetGroundTruth(vtkSmartPointer<vtkImageData> imageData){    
    m_gt_imageData = imageData;    

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

    //Image
    if(m_gt_lut == NULL){
        m_gt_lut = vtkSmartPointer<vtkLookupTable>::New();
        m_gt_lut->SetTableRange(0.0, 2.0);
        m_gt_lut->SetHueRange(0.7, 0.0);
        m_gt_lut->Build();
    }
    
    if(m_gt_imageProperty == NULL){
        m_gt_imageProperty = vtkSmartPointer<vtkImageProperty>::New();
        m_gt_imageProperty->SetInterpolationTypeToLinear();                
        m_gt_imageProperty->SetColorLevel(1.0);
        m_gt_imageProperty->SetColorWindow(1.0);
        m_gt_imageProperty->SetOpacity(0.3);
        m_gt_imageProperty->SetLookupTable(m_gt_lut);
    }
    

    if(m_gt_imageSlice[0] == NULL){
        for(int i=0 ; i<3 ; i++){
            m_gt_sliceMapper[i] = vtkSmartPointer<vtkImageSliceMapper>::New();
            m_gt_sliceMapper[i]->SetOrientation(i);            

            m_gt_imageSlice[i] = vtkSmartPointer<vtkImageSlice>::New();
            m_gt_imageSlice[i]->SetProperty(m_gt_imageProperty);
            m_gt_imageSlice[i]->SetMapper(m_gt_sliceMapper[i]);
            
            double* position = m_gt_imageSlice[i]->GetPosition();
            switch(i){
                case AXL:
                m_gt_imageSlice[i]->SetPosition(position[0]-10, position[1], position[2]);
                break;
                case COR:
                m_gt_imageSlice[i]->SetPosition(position[0], position[1]+10, position[2]);
                break;
                case SAG:
                break;
                m_gt_imageSlice[i]->SetPosition(position[0], position[1], position[2]-10);
                default:
                break;
            }
            
        }
    }

    for(int i=0 ; i<3 ; i++){
        int sliceNum = m_gt_sliceMapper[i]->GetSliceNumberMaxValue() / 2;
        m_gt_sliceMapper[i]->SetSliceNumber(sliceNum);
    }
    
    m_gt_volumeMapper->SetInputData(m_gt_imageData);
    m_gt_volumeMapper->Update();

    m_gt_sliceMapper[0]->SetInputData(m_gt_imageData);
    m_gt_sliceMapper[0]->Update();

    m_gt_sliceMapper[1]->SetInputData(m_gt_imageData);
    m_gt_sliceMapper[1]->Update();

    m_gt_sliceMapper[2]->SetInputData(m_gt_imageData);
    m_gt_sliceMapper[2]->Update();

    
}

void E_Volume::Update(){

    if(m_gt_volumeMapper == NULL) return;

    m_gt_volume->Update();

    if(m_gt_sliceMapper[0] == NULL) return;
    
    m_gt_imageSlice[0]->Update();
    m_gt_imageSlice[1]->Update();
    m_gt_imageSlice[2]->Update();

}

void E_Volume::AssignGroundTruthVolume(int slice, tensorflow::Tensor tensorImage){
    if(m_gt_imageData == NULL) return;

    // Deepcopy - assign - reset ground truth (to avoid VTK bug...)

    
    //Assign to ground-truth imagedata
    
    // auto tensorImageMapper = tensorImage.tensor<float,3>();
    // for(int i=0 ; i<tensorImage.dim_size(1) ; i++){
    //     for(int j=0 ; j<tensorImage.dim_size(2) ; j++){
    //         std::cout << tensorImageMapper(0, i, j);
    //     }
    // }

    // for(int y=0 ; y<dims[1] ; y++){
    //     for(int x=0 ; x<dims[0] ; x++){
    //         float* pointer = static_cast<float*>(imageData->GetScalarPointer(x+extent[0], y+extent[2], slice));            
    //         pointer[0] = tensorImageMapper(0, x, y);            
    //     }
    // }

    //memcpy(m_gt_imageData->GetScalarPointer(0, slice, 0), tensorImage.tensor<float,3>().data(), tensorImage.TotalBytes());

    //m_gt_imageData->DeepCopy(imageData);
    // SetGroundTruth(imageData);

}


void E_Volume::SetSlice(int idx, int sliceNum){
    if(m_sliceMapper[idx] == NULL) return;
    m_sliceMapper[idx]->SetSliceNumber(sliceNum);

    if(m_gt_sliceMapper[idx] == NULL) return;
    m_gt_sliceMapper[idx]->SetSliceNumber(sliceNum);
    
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
    double huRange[2] = {-1024, 3096};
    
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