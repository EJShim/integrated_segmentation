#include "E_TreeWidgetVolume.h"
#include "E_Manager.h"
#include <QAction>
#include <QMenu>
#include <QPoint>
#include <QSignalMapper>
#include <QFileDialog>
#include <QHeaderView>

E_TreeWidgetVolume::E_TreeWidgetVolume(QWidget *parent){
    
    m_currentParentIdx = -1;
    m_currentChildIdx = -1;


    Initialize();
}


E_TreeWidgetVolume::~E_TreeWidgetVolume(){

}


void E_TreeWidgetVolume::Initialize(){

    setSortingEnabled(false);
    setAlternatingRowColors(true);
    setHeaderHidden(true);

    setColumnCount(3);
    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    // QStringList headerLabels;
    // headerLabels.push_back(tr("description"));
    // headerLabels.push_back(tr("slice"));
    // headerLabels.push_back(tr("ground truth"));
    // setHeaderLabels(headerLabels);

    setSelectionBehavior(QAbstractItemView::SelectItems);

    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem*, int)));
    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(onItemChanged(QTreeWidgetItem*, int)));

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
        // toplevelItem->setText(1, "");
        for(int j=0 ; j<columnCount() ; j++){
            toplevelItem->setBackground(j, QBrush(QColor("blue")));
        }

        addTopLevelItem(toplevelItem);

        ///Add Children
        for(int j=0 ; j<toplevelData->GetNumberOfSerieses() ; j++){
            QTreeWidgetItem* childItem = new QTreeWidgetItem();
            childItem->setText(0, toplevelData->GetSeriesDescription(j).c_str());            
            if(toplevelData->IsGroundTruthExist(j)){
                // childItem->setFlags(childItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
                childItem->setCheckState(2, Qt::Unchecked);
            }
            toplevelItem->addChild(childItem);
        }
    }

    //Expand All
    expandAll();
}


/////////////////////////////////////////////////////////////////

 void E_TreeWidgetVolume::onItemDoubleClicked(QTreeWidgetItem* item, int column){
     if(column != 0) return;

    if(item->parent() == NULL){
        return;
    }

    int parentIdx = indexOfTopLevelItem(item->parent());
    int childIdx = item->parent()->indexOfChild(item);

    E_Manager::VolumeMgr()->AddSelectedVolume(parentIdx, childIdx);
 }

void E_TreeWidgetVolume::onItemChanged(QTreeWidgetItem* item, int column){
    if(column != 2) return;

    if(item->parent() == NULL){
        return;
    }

    int parentIdx = indexOfTopLevelItem(item->parent());
    int childIdx = item->parent()->indexOfChild(item);


    if(item->checkState(2)){
        E_Manager::Mgr()->SetLog("Add Ground Truth",NULL);
        E_Manager::VolumeMgr()->AddGroundTruth(parentIdx, childIdx);
    }else{
        E_Manager::Mgr()->SetLog("Remove Ground Truth",NULL);
        E_Manager::VolumeMgr()->RemoveGroundTruth();
    }



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
    }

    Update();
}