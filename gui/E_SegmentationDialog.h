#pragma once

#include <QDialog>
#include <QToolBar>
#include <QSlider>

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
    void UpdateSlider(int len);

    protected:
    QSlider* m_sliceSlider;

    public Q_SLOTS:
    void onSliderChange(int);


};