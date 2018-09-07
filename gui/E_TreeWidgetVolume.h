#pragma once

#include <QTreeWidget>


class E_TreeWidgetVolume : public QTreeWidget{
    Q_OBJECT
    
    public:
    E_TreeWidgetVolume(QWidget *parent = 0);
    ~E_TreeWidgetVolume();


    protected:
    void Initialize();


    public:
    void Update();


    public Q_SLOTS:
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void prepareMenu(const QPoint& pos);

};