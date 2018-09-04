#include "E_ContextInteractorStyle.h"
#include "E_Manager.h"
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>

E_ContextInteractorStyle::E_ContextInteractorStyle(){
        
}

E_ContextInteractorStyle::~E_ContextInteractorStyle(){

}


void E_ContextInteractorStyle::OnMouseMove()
{
    std::cout << "Mouse Move " << std::endl;
}