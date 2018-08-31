#ifndef E_SEGMENTATIONTHREAD_H
#define E_SEGMENTATIONTHREAD_H

#include <QObject>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>

#include "tensorflow/core/public/session.h"
#include "tensorflow/core/framework/tensor.h"



class E_SegmentationThread : public QObject{    
    Q_OBJECT
    public:
    E_SegmentationThread();
    ~E_SegmentationThread();

    public slots:
    void process();

    signals:
    void finished();
    void onCalculated(int i, tensorflow::Tensor);

    private:
    // tensorflow::Session m_session;
    vtkSmartPointer<vtkImageData> m_imageData;
    int* m_dims;

    public:
    void SetImageData(vtkSmartPointer<vtkImageData> data);
    
    
    private:
    vtkSmartPointer<vtkImageData> GetSingleBatchImage(int slice);
    tensorflow::Tensor ConvertImageToTensor(vtkSmartPointer<vtkImageData> input);
    


    
};



#endif