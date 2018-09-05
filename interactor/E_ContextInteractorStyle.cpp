#include "E_ContextInteractorStyle.h"
#include "E_Manager.h"
#include <vtkCommand.h>

E_ContextInteractorStyle::E_ContextInteractorStyle(){
        m_bLButtonDown = false;
}

E_ContextInteractorStyle::~E_ContextInteractorStyle(){

}
void E_ContextInteractorStyle::OnLeftButtonDown(){
    vtkContextInteractorStyle::OnLeftButtonDown();
    m_bLButtonDown = true;
}

void E_ContextInteractorStyle::OnLeftButtonUp(){
    vtkContextInteractorStyle::OnLeftButtonUp();
    m_bLButtonDown = false;
}

void E_ContextInteractorStyle::OnMouseMove()
{
    vtkContextInteractorStyle::OnMouseMove();
    if(m_bLButtonDown){
        E_Manager::Mgr()->RedrawAll();
    }
}