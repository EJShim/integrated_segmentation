#pragma once

#include <vtkSmartPointer.h>
#include <vtkInteractorStyleUser.h>
#include <vtkContextInteractorStyle.h>

class E_ContextInteractorStyle : public vtkContextInteractorStyle{

    public:
    vtkTypeMacro(E_ContextInteractorStyle, vtkContextInteractorStyle);
    static E_ContextInteractorStyle *New() {return new E_ContextInteractorStyle;}

    public:
    E_ContextInteractorStyle();
    ~E_ContextInteractorStyle();

    protected:
    bool m_bLButtonDown;


    public:
    // virtual void OnMouseWheelForward();
    // virtual void OnMouseWheelBackward();
    virtual void OnLeftButtonDown();
    virtual void OnLeftButtonUp();
    virtual void OnMouseMove();

};