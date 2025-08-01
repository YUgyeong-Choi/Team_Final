#include "Observer_Player_Status.h"

CObserver_Player_Status::CObserver_Player_Status()
{
}

void CObserver_Player_Status::OnNotify(const _wstring& eventType, void* data)
{
    bool isCurrent = eventType.find(L"Current") != eventType.npos;
    bool isMax = eventType.find(L"Max") != eventType.npos;

    if (!isCurrent && !isMax)
        return;

    if (eventType.find(L"HP") != eventType.npos)
    {
        if (isCurrent)
        {
           
        }
        else if (isMax)
        {
            
        }
    }
    else if (eventType.find(L"MP") != eventType.npos)
    {
        if (isCurrent)
        {
            
        }
        else if (isMax)
        {
            
        }
    }
    else if (eventType.find(L"Stamina") != eventType.npos)
    {
        if (isCurrent)
        {
           
        }
        else if (isMax)
        {
           
        }
    }
}


void CObserver_Player_Status::Reset()
{

	m_fHpRatio = 1.f;
	m_fMpRatio = 1.f;
	m_fStaminaRatio = 1.f;


}

void CObserver_Player_Status::Free()
{
	__super::Free();

}
