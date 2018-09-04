#pragma once

#include <vtkSmartPointer.h>
#include <vtkContextInteractorStyle.h>


class E_ContextInteractorStyle : public vtkContextInteractorStyle{

    public:
    vtkTypeMacro(E_ContextInteractorStyle, vtkContextInteractorStyle);
    static E_ContextInteractorStyle *New() {return new E_ContextInteractorStyle;}

    public:
    E_ContextInteractorStyle();
    ~E_ContextInteractorStyle();


    public:
    // virtual void OnMouseWheelForward();
    // virtual void OnMouseWheelBackward();
    // virtual void OnLeftButtonDown();
    // virtual void OnLeftButtonUp();
    virtual void OnMouseMove();

};