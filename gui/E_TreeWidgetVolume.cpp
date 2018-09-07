#include "E_TreeWidgetVolume.h"
#include "E_Manager.h"
#include <QAction>
#include <QMenu>
#include <QPoint>

E_TreeWidgetVolume::E_TreeWidgetVolume(QWidget *parent){

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
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SLOT(customContextMenuRequested(const QPoint &pos)), this, SLOT(prepareMenu(const QPoint &pos)));
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


 void E_TreeWidgetVolume::prepareMenu( const QPoint & pos )
{
    std::cout << "right clicked?? " << std::endl;

    QTreeWidgetItem *nd = this->itemAt( pos );



    QAction *newAct = new QAction("test menu", this);
    newAct->setStatusTip(tr("new sth"));
    //onnect(newAct, SIGNAL(triggered()), this, SLOT(newDev()));


    QMenu menu(this);
    menu.addAction(newAct);

    QPoint pt(pos);
    menu.exec( this->mapToGlobal(pos) );
}