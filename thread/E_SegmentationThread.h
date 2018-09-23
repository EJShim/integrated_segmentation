#ifndef E_SEGMENTATIONTHREAD_H
#define E_SEGMENTATIONTHREAD_H

#include <QObject>
#include <itkImage.h>

#include "tensorflow/core/public/session.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/cc/saved_model/loader.h"



class E_SegmentationThread : public QObject{    
    Q_OBJECT
    public:
    typedef itk::Image<float, 3> ImageType;

    E_SegmentationThread();
    ~E_SegmentationThread();

    protected:
    void Initialize();

    public slots:
    void process();

    signals:
    void finished();
    void onCalculated(int i);

    protected:
    // tensorflow::Session m_session;
    ImageType::Pointer m_imageData;

    // Get Current Working index
    int m_patientIdx;
    int m_sereisIdx;

    tensorflow::Session* m_session;
    tensorflow::SavedModelBundle m_sessionContainer;

    public:
    void SetImageData(ImageType::Pointer data);
    void SetTargetData(int patientIdx, int seriesIdx);
    
    
    protected:
    ImageType::Pointer GetSlice(int startIdx);

    //Assign Calculated Ground Truth
    void AssignGroundTruth(int idx, tensorflow::Tensor tensor);
};



#endif