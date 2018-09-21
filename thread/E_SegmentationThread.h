#ifndef E_SEGMENTATIONTHREAD_H
#define E_SEGMENTATIONTHREAD_H

#include <QObject>
#include <itkImage.h>

#include "tensorflow/core/public/session.h"
#include "tensorflow/core/framework/tensor.h"



class E_SegmentationThread : public QObject{    
    Q_OBJECT
    public:
    typedef itk::Image<float, 3> ImageType;

    E_SegmentationThread();
    ~E_SegmentationThread();

    public slots:
    void process();

    signals:
    void finished();
    void onCalculated(int i, tensorflow::Tensor);

    private:
    // tensorflow::Session m_session;
    ImageType::Pointer m_imageData;

    public:
    void SetImageData(ImageType::Pointer data);
    
    
    private:
    ImageType::Pointer GetSlice(int startIdx);
    // vtkSmartPointer<vtkImageData> GetSingleBatchImage(int slice);
    // tensorflow::Tensor ConvertImageToTensor(vtkSmartPointer<vtkImageData> input);
    


    
};



#endif