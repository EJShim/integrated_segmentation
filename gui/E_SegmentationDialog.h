#pragma once

#include <QDialog>
#include <QToolBar>
#include <QSlider>
#include "E_SegmentationThread.h"


class E_SegmentationDialog : public QDialog{
    Q_OBJECT

    public:
    E_SegmentationDialog(QWidget *parent = 0);

    protected:
    void Initialize();
    QWidget* RendererWidgets();
    QWidget* ButtonWidgets();
    QToolBar* LowerToolbar();

    public:

    ///Initialize Network
    void InitializeNetwork();

    ///Update Slider
    void UpdateSlider(int len);

    protected:
    ///Segmentation Worker    
    E_SegmentationThread* m_segmentationWorker;

    //Slider
    QSlider* m_sliceSlider;

    public Q_SLOTS:
    void onSliderChange(int);
    void onStartSegmentation();
    void onProgressSegmentation(int);
    void onFinishSegmentation();
    void onSaveGroundTruth();

    void onClose(int result);


};