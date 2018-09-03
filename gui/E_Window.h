#include <QMainWindow>
#include <QToolBar>
#include <QVTKOpenGLWidget.h>
#include <QGroupBox>
#include <QCheckBox>
#include <QDockWidget>
#include <QListWidget>
#include <QTreeWidget>
#include "tensorflow/core/framework/tensor.h"

#include "E_Manager.h"
#include "E_SegmentationThread.h"




class E_Window : public QMainWindow{
    Q_OBJECT
    
    public:
    E_Window(QWidget *parent = 0);
    ~E_Window();
    
    protected:
    ///Rendering Widget
    QVTKOpenGLWidget* m_renderingWidget[E_Manager::NUM_VIEW];


    ///Checkboxex for 3d slice
    QCheckBox* m_checkboxAxl;
    QCheckBox* m_checkboxCor;
    QCheckBox* m_checkboxSag;


    ///Volume Tree Widget
    QDockWidget* m_volumeTreeDocker;
    QTreeWidget* m_volumeTreeWidget;
    


    /// Histogram Widget
    QDockWidget* m_histDocker;
    QVTKOpenGLWidget* m_histogramWidget;

    ///Log Widget
    QDockWidget* m_logDocker;
    QListWidget* m_logWidget;

    protected:
    QToolBar* InitToolbar();
    QWidget* InitCentralWidget();
    QGroupBox* Init3DSliceGroup();
    void CreateDockWindows();

    E_SegmentationThread* m_segmentationThread;


    public:

    ///Update Volume Tree
    void UpdateVolumeTree();



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

    ///Tree Widget
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    
};