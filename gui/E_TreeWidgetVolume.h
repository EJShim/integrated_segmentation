#pragma once

#include <QTreeWidget>


class E_TreeWidgetVolume : public QTreeWidget{
    Q_OBJECT
    
    public:
    E_TreeWidgetVolume(QWidget *parent = 0);
    ~E_TreeWidgetVolume();


    protected:
    void Initialize();


    protected:
    int m_currentParentIdx;
    int m_currentChildIdx;


    public:
    void Update();


    public Q_SLOTS:
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onItemChanged(QTreeWidgetItem* item, int column);
    void onContextMenu(const QPoint& pos);
    void onImportGroundTruth();

};