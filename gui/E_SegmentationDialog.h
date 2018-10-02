#pragma once

#include <QDialog>

class E_SegmentationDialog : public QDialog{
    Q_OBJECT

    public:
    E_SegmentationDialog(QWidget *parent = 0);

    protected:
    void Initialize();
    QWidget* RendererWidgets();
    QWidget* ButtonWidgets();


};