#include "E_Window.h"
#include <QApplication>
#include <QDesktopWidget>
#include <iostream>
#include <E_DicomSeries.h>
#include <QGridLayout>
#include <QAction>
#include <QFileDialog>
#include <QFileInfo>
#include <QTimer>
#include <QSettings>
#include <QThread>
#include <math.h>

E_Window::E_Window(QWidget* parent):QMainWindow(parent){

    //initialize screen size
    QRect rec = QApplication::desktop()->screenGeometry();
    int height = rec.height();
    int width = rec.width();
    m_screenSize = sqrt(pow(height, 2) + pow(width, 2));

    // Initialize toolbar
    this->addToolBar(Qt::TopToolBarArea, this->InitToolbar());
    
    //Initialize Central WIdget    
    this->setCentralWidget(this->InitCentralWidget());

    // Add Dock widget
    this->CreateDockWindows();

    m_segmentationWorker = NULL;

    this->showMaximized();
}

E_Window::~E_Window(){
        
}

QToolBar* E_Window::InitToolbar(){

    QRect rec = QApplication::desktop()->screenGeometry();
    int height = rec.height();
    int iconSize = m_screenSize / 50;

    //Initialize Toolbar
    QToolBar* toolbar = new QToolBar();
    toolbar->setIconSize(QSize(iconSize, iconSize));
    toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->setMovable(false);    

    //Import Volume    
    QAction* import_action = new QAction(QIcon(":/images/pantone-2.png") ,QString("Import Volume"), this);
    toolbar->addAction(import_action);
    connect(import_action, SIGNAL(triggered()), this, SLOT(ImportVolume()));    


    //Run Segmentation
    QAction* segmentation_action = new QAction(QIcon(":/images/edit-tools.png") ,QString("Run Segmentation"), this);
    toolbar->addAction(segmentation_action);
    connect(segmentation_action, SIGNAL(triggered()), this, SLOT(RunSegmentation()));

    // toolbar->addWidget(Init3DSliceGroup());

    return toolbar;
}

QGroupBox* E_Window::Init3DSliceGroup(){

    QGroupBox* groupbox = new QGroupBox(tr("3d slice"));
    groupbox->setFlat(true);

    m_checkboxAxl = new QCheckBox(tr("Axial"));
    m_checkboxCor = new QCheckBox(tr("Coronal"));
    m_checkboxSag = new QCheckBox(tr("Sagittal"));

    m_checkboxAxl->setChecked(true);
    m_checkboxCor->setChecked(true);
    m_checkboxSag->setChecked(true);

    connect(m_checkboxAxl, SIGNAL(stateChanged(int)), this, SLOT(ToggleAxlSlice(int)));
    connect(m_checkboxCor, SIGNAL(stateChanged(int)), this, SLOT(ToggleCorSlice(int)));
    connect(m_checkboxSag, SIGNAL(stateChanged(int)), this, SLOT(ToggleSagSlice(int)));

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(m_checkboxAxl);
    vbox->addWidget(m_checkboxCor);
    vbox->addWidget(m_checkboxSag);    
    groupbox->setLayout(vbox);


    return groupbox;
}

QWidget* E_Window::InitCentralWidget(){
    //Initialize VTK Widget
    for(int i=0 ; i<E_Manager::NUM_VIEW ; i++){
        this->m_renderingWidget[i] = new QVTKOpenGLWidget();        
        E_Manager::Mgr()->SetVTKWidget(this->m_renderingWidget[i],i);


        #ifdef __APPLE__
            //Force to use GL>3.2,, mac default is 2.1
            QSurfaceFormat::setDefaultFormat(m_renderingWidget[i]->defaultFormat());
            m_renderingWidget[i]->setFormat(m_renderingWidget[i]->defaultFormat());        
        #endif
    }

    
    
    // Initialize Central Widgets
    QWidget* centralWidget = new QWidget();
    QGridLayout* layout = new QGridLayout();
    centralWidget->setLayout(layout);

    layout->setSpacing(1);
    layout->setContentsMargins(0, 0, 0, 0);


    //Add To Central Widget(Grid Layout)
    layout->addWidget(this->m_renderingWidget[E_Manager::VIEW_MAIN], 0, 1);
    layout->addWidget(this->m_renderingWidget[E_Manager::VIEW_AXL], 0, 2);
    layout->addWidget(this->m_renderingWidget[E_Manager::VIEW_COR], 1, 1);
    layout->addWidget(this->m_renderingWidget[E_Manager::VIEW_SAG], 1, 2);


    return centralWidget;
}


void E_Window::CreateDockWindows(){

    //////////Volume Tree Widget
    m_volumeTreeDocker = new QDockWidget("Patient Series", this);
    m_volumeTreeDocker->setAllowedAreas(Qt::LeftDockWidgetArea);
    this->addDockWidget(Qt::LeftDockWidgetArea, m_volumeTreeDocker);

    //Add Tree Widget To Docker
    m_volumeTreeWidget = new E_TreeWidgetVolume(this);
    m_volumeTreeDocker->setWidget(m_volumeTreeWidget);


    //////////// Volume Info Widget



    /////// Histogram Widget
    m_histDocker = new QDockWidget("histogram", this);
    m_histDocker->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);

    //Create Histogram Widget
    m_histogramWidget = new E_HistogramWidget();
    // m_histogramWidget->setMinimumHeight(int(m_screenSize/20));
    E_Manager::Mgr()->SetHistogramWidget(m_histogramWidget->GetRenderer());
    E_Manager::VolumeMgr()->SetComboBox(m_histogramWidget->GetComboBoxController());
    m_histDocker->setWidget(m_histogramWidget);
    

    // Add To Mainwindow
    this->addDockWidget(Qt::LeftDockWidgetArea, m_histDocker);



    // Test Another Dock Widget
    m_logDocker = new QDockWidget("log", this);
    m_logDocker->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    E_Manager::Mgr()->SetLogWidget(m_logDocker);
    //Add To MainWindow
    this->addDockWidget(Qt::LeftDockWidgetArea, m_logDocker);
}


void E_Window::UpdateVolumeTree(){
    m_volumeTreeWidget->Update();
}


////////////////////////////////////////////////////////////////////Action SLOTS////////////////////////////////////////////////////////
void E_Window::ImportVolume(){

    const QString DEFAULT_DIR_KEY("default_dir");
    QSettings MySettings;
    QString fileName = QFileDialog::getOpenFileName(this, ("Open File"),  MySettings.value(DEFAULT_DIR_KEY).toString() , tr("Dicom file(*.dcm) ;; NII file(*.nii)"));

    if(fileName.length() < 1) return;    
    MySettings.setValue(DEFAULT_DIR_KEY,  QDir(fileName).absolutePath());
    QFileInfo info(fileName);
    QString ext = info.completeSuffix();


    // Import Volume
    if(ext == "nii"){
        E_Manager::Mgr()->SetLog("Import *.nii File, this is deprecated", NULL);
    }
    else if(ext == "dcm"){
        E_Manager::Mgr()->SetLog("Import DICOM(*.dcm) file", NULL);
        QDir directoryPath = info.dir();
        E_Manager::VolumeMgr()->ImportDicom(directoryPath.absolutePath().toLocal8Bit().data());

        UpdateVolumeTree();
    }


}

void E_Window::RunSegmentation(){

    //Make Blank Ground Truth
    E_Manager::VolumeMgr()->MakeBlankGroundTruth();
    m_volumeTreeWidget->Update();


    //Initialize Segmentation Thread.
    if(m_segmentationWorker == NULL){

        //Initialize Thread        
        m_segmentationWorker = new E_SegmentationThread();
        connect(m_segmentationWorker, SIGNAL(onCalculated(int)), this, SLOT(OnSegmentationCalculated(int)));
        connect(m_segmentationWorker, SIGNAL(finished()), this, SLOT(OnFinishedSegmentation()));
        
    }
    //Move To Thread
    QThread* thread = new QThread;
    connect(thread, SIGNAL(started()), m_segmentationWorker, SLOT(process()));
    m_segmentationWorker->moveToThread(thread);
    


    ///Set Patient Index here
    thread->start();
}

void E_Window::OnSegmentationCalculated(int i){
    
    // Progress
    E_Manager::VolumeMgr()->GetCurrentVolume()->SetSlice(1, i);
    E_Manager::Mgr()->SetLog("processing segmentation ", std::to_string(i).c_str() , NULL);

    E_Manager::Mgr()->RedrawAll(false);
    
}

void E_Window::OnFinishedSegmentation(){
    E_Manager::Mgr()->SetLog("Segmentation Finihsed! ", NULL);
}

void E_Window::ToggleAxlSlice(int state){
    E_Manager::VolumeMgr()->Toggle3DSlice(0, state);
}

void E_Window::ToggleCorSlice(int state){
    E_Manager::VolumeMgr()->Toggle3DSlice(1, state);
}

void E_Window::ToggleSagSlice(int state){
    E_Manager::VolumeMgr()->Toggle3DSlice(2, state);
}

void E_Window::OnTimeOut(){
    E_Manager::Mgr()->Redraw(0);
    E_Manager::Mgr()->Redraw(3);
}



