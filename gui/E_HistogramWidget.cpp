#include "E_HistogramWidget.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QRect>
#include <QVBoxLayout>
#include <QFrame>
#include <E_Manager.h>

E_HistogramWidget::E_HistogramWidget(QWidget *parent){

    Initialize();
}


E_HistogramWidget::~E_HistogramWidget(){


}

void E_HistogramWidget::Initialize(){
    
    //Add Renderer To Current Widget
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    
    InitializeDropdownMenu();

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);


    InitializeRenderer();
}

void E_HistogramWidget::InitializeDropdownMenu(){
    
    
    m_comboBox = new QComboBox();
    
    
    m_comboBox->addItem("MIP");
    m_comboBox->addItem("GroundTruth");

    layout()->addWidget(m_comboBox);
    // m_comboBox->setEnabled(false);


    connect(m_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onIndexChanged(int)));
}

void E_HistogramWidget::InitializeRenderer(){
    QRect rec = QApplication::desktop()->screenGeometry();
    int height = rec.height();
    int width = rec.width();
    double screenSize = sqrt(pow(height, 2) + pow(width, 2));

    m_renderer = new QVTKOpenGLWidget();
    m_renderer->setMinimumHeight(int(screenSize/20));

    layout()->addWidget(m_renderer);
}



/////////////SLOTS//////

void E_HistogramWidget::onIndexChanged(int idx){
    
    E_Volume* volume = E_Manager::VolumeMgr()->GetCurrentVolume();
    if(volume == NULL) {
        E_Manager::Mgr()->SetLog("Add Volume First", NULL);
        return;
    }

    E_Manager::Mgr()->SetLog("Transfer Function Changed", std::to_string(idx).c_str());
    volume->SetTransferFunction(idx);
    E_Manager::Mgr()->RedrawAll(false);
}