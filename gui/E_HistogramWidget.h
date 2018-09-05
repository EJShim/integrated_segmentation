#pragma once

#include <QWidget>
#include <QVTKOpenGLWidget.h>
#include <QComboBox>

class E_HistogramWidget : public QWidget{
    Q_OBJECT
    
    public:
    E_HistogramWidget(QWidget *parent = 0);
    ~E_HistogramWidget();

    protected:
    void Initialize();
    void InitializeDropdownMenu();
    void InitializeRenderer();


    protected:
    QVTKOpenGLWidget* m_renderer;
    QComboBox* m_comboBox;


    public:
    QVTKOpenGLWidget* GetRenderer(){return m_renderer;}
    QComboBox* GetComboBoxController(){return m_comboBox;}
    public Q_SLOTS:
    void onIndexChanged(int idx);
};