#ifndef E_INTERACTORSTYLE_H
#define E_INTERACTORSTYLE_H
#include <vtkSmartPointer.h>
#include <vtkInteractorStyleImage.h>

class E_InteractorStyle : public vtkInteractorStyleImage{
    public:
    vtkTypeMacro(E_InteractorStyle, vtkInteractorStyleImage);
    static E_InteractorStyle *New() {return new E_InteractorStyle;}

    public:
    E_InteractorStyle();
    ~E_InteractorStyle();

    protected:
    int m_idx;

    public:
    void SetIdx(int idx){m_idx = idx;}

    virtual void OnMouseWheelForward();
    virtual void OnMouseWheelBackward();
    virtual void OnLeftButtonDown();
    virtual void OnLeftButtonUp();
    virtual void OnMouseMove();

};

#endif //E_INTERACTORSTYLE