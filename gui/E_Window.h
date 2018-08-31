#include <QMainWindow>
#include <QToolBar>
#include <QVTKOpenGLWidget.h>
#include <QGroupBox>
#include <QCheckBox>

#include "tensorflow/core/framework/tensor.h"

#include "E_Manager.h"
#include "E_SegmentationThread.h"



class E_Window : public QMainWindow{
    Q_OBJECT
    
    public:
    E_Window(QWidget *parent = 0);
    ~E_Window();
    
    protected:
    QVTKOpenGLWidget* m_renderingWidget[E_Manager::NUM_VIEW];

    ///Checkboxex for 3d slice
    QCheckBox* m_checkboxAxl;
    QCheckBox* m_checkboxCor;
    QCheckBox* m_checkboxSag;

    protected:
    QToolBar* InitToolbar();
    QWidget* InitCentralWidget();
    QGroupBox* Init3DSliceGroup();

    E_SegmentationThread* m_segmentationThread;



    public Q_SLOTS:
    void OnTimeOut();

    void ImportVolume();
    void RunSegmentation();
    void OnSegmentationCalculated(int i, tensorflow::Tensor);
    void OnFinishedSegmentation();

    void ToggleAxlSlice(int state);
    void ToggleCorSlice(int state);
    void ToggleSagSlice(int state);
    ///temporary
    void ImportGT();
    
};