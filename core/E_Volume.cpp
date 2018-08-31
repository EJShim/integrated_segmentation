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
    
}


E_Volume::~E_Volume(){
    
}

void E_Volume::SetImageData(vtkSmartPointer<vtkImageData> imageData){
    if(m_imageData == NULL) m_imageData = vtkSmartPointer<vtkImageData>::New();
    
    m_imageData->DeepCopy(imageData);

    double* scalarRange = m_imageData->GetScalarRange();

    if(m_colorFunction == NULL){
        m_colorFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    }else{
        m_colorFunction->RemoveAllPoints();        
    }
    m_colorFunction->AddRGBPoint(scalarRange[0], 0.0, 0.0, 0.0);
    m_colorFunction->AddRGBPoint(scalarRange[1], 1.0, 1.0, 1.0);


    if(m_opacityFunction == NULL){
        m_opacityFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
    }else{
        m_opacityFunction->RemoveAllPoints();
    }
    m_opacityFunction->AddPoint(scalarRange[0], 0.0);
    m_opacityFunction->AddPoint(scalarRange[1], 1.0);

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
        m_volumeMapper->SetInputData(m_imageData);        
        m_volumeMapper->SetBlendModeToMaximumIntensity();        

        this->SetMapper(m_volumeMapper);
    }else{        
        this->Update();
    }

    //Image
    if(m_imageProperty == NULL){
        m_imageProperty = vtkSmartPointer<vtkImageProperty>::New();
        m_imageProperty->SetInterpolationTypeToLinear();
    }
    m_imageProperty->SetColorLevel( (scalarRange[1]+scalarRange[0])/2.0 );
    m_imageProperty->SetColorWindow(scalarRange[1] - scalarRange[0] - 1.0);

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

    if(m_gt_imageData == NULL){
        m_gt_imageData = vtkSmartPointer<vtkImageData>::New();
    }
    m_gt_imageData->DeepCopy(imageData);

    if(m_gt_volume == NULL){
        m_gt_volume = vtkSmartPointer<vtkVolume>::New();
    }


    if(m_gt_colorFunction == NULL){
        m_gt_colorFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    }
    m_gt_colorFunction->AddRGBPoint(0, 0.0, 0.0, 0.0);
    m_gt_colorFunction->AddRGBPoint(1, 0.0, 1.0, 0.0);
    m_gt_colorFunction->AddRGBPoint(2, 1.0, 0.0, 0.0);

    if(m_gt_opacityFunction == NULL){
        m_gt_opacityFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
    }
    m_gt_opacityFunction->AddPoint(0, 0.0);
    m_gt_opacityFunction->AddPoint(1, 0.2);
    m_gt_opacityFunction->AddPoint(2, 1.0);



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
        m_gt_volumeMapper->SetInputData(m_gt_imageData);
        // m_gt_volumeMapper->SetRequestedRenderModeToRayCast();
        m_gt_volumeMapper->SetBlendModeToComposite();

        m_gt_volume->SetMapper(m_gt_volumeMapper);
    }else{                
        // m_gt_volume->Update();
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
            m_gt_sliceMapper[i]->SetInputData(m_gt_imageData);

            m_gt_imageSlice[i] = vtkSmartPointer<vtkImageSlice>::New();
            m_gt_imageSlice[i]->SetProperty(m_gt_imageProperty);
            m_gt_imageSlice[i]->SetMapper(m_gt_sliceMapper[i]);
        }        
    }

    for(int i=0 ; i<3 ; i++){
        int sliceNum = m_gt_sliceMapper[i]->GetSliceNumberMaxValue() / 2;
        m_gt_sliceMapper[i]->SetSliceNumber(sliceNum);
    }    
}

void E_Volume::AssignGroundTruthVolume(int slice, tensorflow::Tensor tensorImage){
    if(m_gt_imageData == NULL) return;

    //Deepcopy - assign - reset ground truth (to avoid VTK bug...)
    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    imageData->DeepCopy(m_gt_imageData);
        
    int* dims = imageData->GetDimensions();
    int* extent = imageData->GetExtent();

    //Assign to ground-truth imagedata
    auto tensorImageMapper = tensorImage.tensor<int,3>();
    for(int y=0 ; y<dims[1] ; y++){
        for(int x=0 ; x<dims[0] ; x++){
            float* pointer = static_cast<float*>(imageData->GetScalarPointer(x+extent[0], y+extent[2], slice));            
            pointer[0] = tensorImageMapper(0, x, y);            
        }
    }

    m_gt_imageData->DeepCopy(imageData);
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
