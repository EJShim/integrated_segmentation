#include "E_ContextInteractorStyle.h"
#include "E_Manager.h"

E_ContextInteractorStyle::E_ContextInteractorStyle(){
    std::cout << "Context" << std::endl;
}

E_ContextInteractorStyle::~E_ContextInteractorStyle(){

}

void E_ContextInteractorStyle::OnMouseWheelForward(){

    std::cout << "wheel " << std::endl;
}

void E_ContextInteractorStyle::OnMouseMove()
{
    std::cout << "mouse move!" << std::endl;
    
}