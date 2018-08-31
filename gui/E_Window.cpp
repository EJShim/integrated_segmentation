#include "E_Window.h"

#include <iostream>
#include <QGridLayout>
#include <QAction>
#include <QFileDialog>
#include <QFileInfo>
#include <QTimer>
#include <QThread>
#include <QDockWidget>
#include <QListWidget>

E_Window::E_Window(QWidget* parent):QMainWindow(parent){
    //Show in maximum size

    // Initialize toolbar
    this->addToolBar(Qt::TopToolBarArea, this->InitToolbar());
    
    //Initialize Central WIdget    
    this->setCentralWidget(this->InitCentralWidget());

    // Add Dock widget
    this->CreateDockWindows();

    m_segmentationThread = NULL;

    this->showMaximized();
}

E_Window::~E_Window(){
        
}

QToolBar* E_Window::InitToolbar(){
    //Initialize Toolbar
    QToolBar* toolbar = new QToolBar();
    toolbar->setIconSize(QSize(50, 50));
    toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->setMovable(false);    

    //Import Volume    
    QAction* import_action = new QAction(QIcon(":/images/pantone-2.png") ,QString("Import Volume"), this);
    toolbar->addAction(import_action);
    connect(import_action, SIGNAL(triggered()), this, SLOT(ImportVolume()));    
    //Import Ground Truth
    QAction* gt_import_action = new QAction(QIcon(":/images/document.png"), QString("Import Ground Truth(temp)"), this);
    toolbar->addAction(gt_import_action);
    connect(gt_import_action, SIGNAL(triggered()), this, SLOT(ImportGT()));


    //Run Segmentation
    QAction* segmentation_action = new QAction(QIcon(":/images/edit-tools.png") ,QString("Run Segmentation"), this);
    toolbar->addAction(segmentation_action);
    connect(segmentation_action, SIGNAL(triggered()), this, SLOT(RunSegmentation()));

    toolbar->addWidget(Init3DSliceGroup());

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
    }

    #ifdef __APPLE__
        //Force to use GL>3.2,, mac default is 2.1
        QSurfaceFormat::setDefaultFormat(widget->defaultFormat());
        widget->setFormat(widget->defaultFormat());        
    #endif
    
    
    // Initialize Central Widgets
    QWidget* centralWidget = new QWidget();
    QGridLayout* layout = new QGridLayout();
    centralWidget->setLayout(layout);


    //Add To Central Widget(Grid Layout)
    layout->addWidget(this->m_renderingWidget[E_Manager::VIEW_MAIN], 0, 1);
    layout->addWidget(this->m_renderingWidget[E_Manager::VIEW_AXL], 0, 2);
    layout->addWidget(this->m_renderingWidget[E_Manager::VIEW_COR], 1, 1);
    layout->addWidget(this->m_renderingWidget[E_Manager::VIEW_SAG], 1, 2);


    return centralWidget;
}


void E_Window::CreateDockWindows(){
    QDockWidget* histWidget = new QDockWidget("histogram", this);
    histWidget->setAllowedAreas(Qt::RightDockWidgetArea);

    //Create Histogram Widget
    m_histogramWidget = new QVTKOpenGLWidget();
    E_Manager::Mgr()->SetHistogramWidget(m_histogramWidget);
    histWidget->setWidget(m_histogramWidget);
    

    // Add To Mainwindow
    this->addDockWidget(Qt::RightDockWidgetArea, histWidget);



    // Test Another Dock Widget
    QDockWidget* testWidget = new QDockWidget("test widget", this);
    testWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QListWidget* listWidget = new QListWidget();
    listWidget->addItems(QStringList()
            << "Thank you for your payment which we have received today."
            << "Your order has been dispatched and should be with you "
               "within 28 days."
            << "We have dispatched those items that were in stock. The "
               "rest of your order will be dispatched once all the "
               "remaining items have arrived at our warehouse. No "
               "additional shipping charges will be made."
            << "You made a small overpayment (less than $5) which we "
               "will keep on account for you, or return at your request."
            << "You made a small underpayment (less than $1), but we have "
               "sent your order anyway. We'll add this underpayment to "
               "your next bill."
            << "Unfortunately you did not send enough money. Please remit "
               "an additional $. Your order will be dispatched as soon as "
               "the complete amount has been received."
            << "You made an overpayment (more than $5). Do you wish to "
               "buy more items, or should we return the excess to you?"   
    );
    testWidget->setWidget(listWidget);

    //Add To MainWindow
    this->addDockWidget(Qt::RightDockWidgetArea, testWidget);


}



////////////////////////////////////////////////////////////////////Action SLOTS////////////////////////////////////////////////////////
void E_Window::ImportVolume(){    
    
    QString fileName = QFileDialog::getOpenFileName(this, ("Open File"),"~/..", tr("Dicom file(*.dcm) ;; NII file(*.nii)"));

    if(fileName.length() < 1) return;
    QFileInfo info(fileName);
    QString ext = info.completeSuffix();


    // Import Volume
    if(ext == "nii"){
        E_Manager::VolumeMgr()->ImportNII(fileName.toLocal8Bit().data());
    }
    else if(ext == "dcm"){
        std::cout << "import dicom " << std::endl;
        QDir directoryPath = info.dir();
        E_Manager::VolumeMgr()->ImportDicom(directoryPath.absolutePath().toLocal8Bit().data());
    }

}

void E_Window::RunSegmentation(){

    //Make Blank Ground Truth
    E_Manager::VolumeMgr()->MakeBlankGroundTruth();    

    //Update Rendering 3d slice
    m_checkboxAxl->setCheckState(Qt::Unchecked);
    m_checkboxCor->setCheckState(Qt::Unchecked);
    m_checkboxSag->setCheckState(Qt::Checked);


    //Set Input Data

    //Initialize Segmentation Thread.
    qRegisterMetaType<tensorflow::Tensor>("tensorflow::Tensor"); 
    QThread* thread = new QThread;
    E_SegmentationThread* segmentationWorker = new E_SegmentationThread();
    segmentationWorker->SetImageData(E_Manager::VolumeMgr()->GetCurrentVolume()->GetImageData());
    segmentationWorker->moveToThread(thread);
    
    connect(thread, SIGNAL(started()), segmentationWorker, SLOT(process()));
    connect(segmentationWorker, SIGNAL(onCalculated(int, tensorflow::Tensor)), this, SLOT(OnSegmentationCalculated(int, tensorflow::Tensor)));
    connect(segmentationWorker, SIGNAL(finished()), this, SLOT(OnFinishedSegmentation()));
    
    
    thread->start();
}

void E_Window::OnSegmentationCalculated(int i, tensorflow::Tensor t){
    
    // Update Animation    
    E_Manager::VolumeMgr()->GetCurrentVolume()->SetSlice(2, i);
    E_Manager::VolumeMgr()->GetCurrentVolume()->AssignGroundTruthVolume(i, t);
    E_Manager::VolumeMgr()->UpdateVolume(E_Manager::VolumeMgr()->GetCurrentVolume()->GetGroundTruthVolume());

    // E_Manager::Mgr()->Redraw(0);
    // E_Manager::Mgr()->Redraw(3);
    E_Manager::Mgr()->RedrawAll();
    
}

void E_Window::OnFinishedSegmentation(){
    std::cout << "segmentation finished" << std::endl;    
}



void E_Window::ImportGT(){
    QString fileName = QFileDialog::getOpenFileName(this, ("Open File"),"~/..",
                                                ("volumes (*.dcm *.nii"));

    if(fileName == "") return;

    E_Manager::VolumeMgr()->ImportGroundTruth(fileName.toStdString());
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