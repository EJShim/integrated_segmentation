#include <QVTKOpenGLWidget.h>
#include <QDockWidget>
#include <QListWidget>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkContextView.h>
#include <vtkChartXY.h>
#include <E_ContextInteractorStyle.h>


#include "E_VolumeManager.h"

class E_Manager{
    public:
    enum {VIEW_MAIN, VIEW_AXL, VIEW_COR, VIEW_SAG};
    static const int NUM_VIEW = 4;

    public:
    E_Manager();
    ~E_Manager();

    private:
    static E_Manager* m_instance;
    static E_VolumeManager* m_volumeManager;

    static void Destroy(){delete m_instance;}
    static void DestroyVolumeManager(){delete m_volumeManager;}

    public:
    static E_Manager* Mgr();
    static E_VolumeManager* VolumeMgr();

    public:
    void Initialize();
    void ClearMemory();

    vtkRenderer* GetRenderer(int idx){return this->m_renderer[idx];}
    vtkChartXY* GetHistogramPlot(){return this->m_histogramPlot;}

    //Initialize VTK Widgets
    void SetVTKWidget(QVTKOpenGLWidget* widget, int idx);

    ///Initialize Histogram Widget
    void SetHistogramWidget(QVTKOpenGLWidget* widget);

    void Redraw(int idx, bool reset=false);
    void RedrawAll(bool reset=false);


    ///Set Log
    void SetLogWidget(QDockWidget* widgetDocker);
    void SetLog(const char *arg, ...);


    protected:
    //Renderer
    vtkSmartPointer<vtkRenderer> m_renderer[NUM_VIEW];

    //Histogram
    vtkSmartPointer<vtkContextView> m_histogramRenderer;
    vtkSmartPointer<vtkChartXY> m_histogramPlot;

    /// Log WIdget
    QListWidget* m_logWidget;
    
};

