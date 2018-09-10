#include "E_TreeWidgetVolume.h"
#include "E_Manager.h"
#include <QAction>
#include <QMenu>
#include <QPoint>
#include <QSignalMapper>
#include <QFileDialog>

E_TreeWidgetVolume::E_TreeWidgetVolume(QWidget *parent){
    
    m_currentParentIdx = -1;
    m_currentChildIdx = -1;


    Initialize();
}


E_TreeWidgetVolume::~E_TreeWidgetVolume(){

}


void E_TreeWidgetVolume::Initialize(){
    setHeaderHidden(true);
    setSortingEnabled(false);
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectItems);

    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem*, int)));

    //Set Policy??
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onContextMenu(const QPoint &)));
}

void E_TreeWidgetVolume::Update(){
      //Clear
    clear();

    std::vector<E_DicomSeries*> patientSeries = E_Manager::VolumeMgr()->GetVolumeList();


    for(int i=0 ; i<patientSeries.size() ; i++){

        ///top level
        E_DicomSeries* toplevelData = patientSeries[i];
        QTreeWidgetItem* toplevelItem = new QTreeWidgetItem();
        
        toplevelItem->setText(0, toplevelData->GetStudyDescription().c_str());
        toplevelItem->setBackground(0, QBrush(QColor("blue")));
        
        addTopLevelItem(toplevelItem);



        ///Add Children
        for(int j=0 ; j<toplevelData->GetNumberOfSerieses() ; j++){
            QTreeWidgetItem* childItem = new QTreeWidgetItem();
            childItem->setText(0, toplevelData->GetSeriesDescription(j).c_str());
            toplevelItem->addChild(childItem);
        }
    }

    //Expand All
    expandAll();
}


/////////////////////////////////////////////////////////////////

 void E_TreeWidgetVolume::onItemDoubleClicked(QTreeWidgetItem* item, int column){

    if(item->parent() == NULL){
        return;
    }

    int parentIdx = indexOfTopLevelItem(item->parent());
    int childIdx = item->parent()->indexOfChild(item);

    E_Manager::VolumeMgr()->AddSelectedVolume(parentIdx, childIdx);

 }


 void E_TreeWidgetVolume::onContextMenu( const QPoint & pos )
{
    ///Get Selected Item
    QTreeWidgetItem *item = this->itemAt( pos );
    if(item == NULL) return;
    if(item->parent() == NULL){
        return;
    }
    m_currentParentIdx = indexOfTopLevelItem(item->parent());
    m_currentChildIdx = item->parent()->indexOfChild(item);


    //Make Import Ground Truth Action
    QAction *action = new QAction("Import Ground Truth", this);
    QSignalMapper* mapper = new QSignalMapper(this);
    connect(action, SIGNAL(triggered()), this, SLOT(onImportGroundTruth()));

    //Pop-up Menu
    QMenu menu(this);
    menu.addAction(action);
    menu.exec( this->mapToGlobal(pos) );
}

void E_TreeWidgetVolume::onImportGroundTruth(){
    if(m_currentParentIdx == -1 || m_currentChildIdx == -1) return;

    QString fileName = QFileDialog::getOpenFileName(this, ("Open File"),"~/..", tr("Dicom file(*.dcm) ;; NII file(*.nii)"));
    if(fileName.length() < 1) return;
    QFileInfo info(fileName);
    QString ext = info.completeSuffix();


    // Import Volume
    if(ext == "nii"){
        E_Manager::Mgr()->SetLog("Import *.nii Ground Truth File of ", std::to_string(m_currentParentIdx), std::to_string(m_currentChildIdx), NULL);

    }
    else if(ext == "dcm"){
        E_Manager::Mgr()->SetLog("Import DICOM(*.dcm) Ground Truth File of ",  std::to_string(m_currentParentIdx), std::to_string(m_currentChildIdx), NULL);
        QDir directoryPath = info.dir();
        E_Manager::VolumeMgr()->ImportGroundTruth(directoryPath.absolutePath().toLocal8Bit().data(), m_currentParentIdx, m_currentChildIdx);

        // UpdateVolumeTree();
    }
}