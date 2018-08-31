#include "E_Manager.h"
#include "E_InteractorStyle.h"


E_InteractorStyle::E_InteractorStyle()
{
    m_idx = NULL;
}

E_InteractorStyle::~E_InteractorStyle()
{
    
}


void E_InteractorStyle::OnMouseWheelForward()
{    
    E_Manager::VolumeMgr()->ForwardSlice(this->m_idx);
}

void E_InteractorStyle::OnMouseWheelBackward()
{
    E_Manager::VolumeMgr()->BackwardSlice(this->m_idx);
}

void E_InteractorStyle::OnLeftButtonDown()
{
    
}

void E_InteractorStyle::OnLeftButtonUp()
{
    
}

void E_InteractorStyle::OnMouseMove()
{
    
}