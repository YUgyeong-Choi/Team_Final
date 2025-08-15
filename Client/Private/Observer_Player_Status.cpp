#include "Observer_Player_Status.h"

CObserver_Player_Status::CObserver_Player_Status()
{
	Register_PullCallback([this](_wstring eventType, void* data) {
		if (L"CurrentHP" == eventType)
			m_iCurrentHP = *static_cast<int*>(data);
		else if (L"MaxHP" == eventType)
			m_iMaxHP = *static_cast<int*>(data);
		});

	Register_PullCallback([this](_wstring eventType, void* data) {
		if (L"CurrentStamina" == eventType)
			m_fCurrentStamina = *static_cast<_float*>(data);
		else if (L"MaxStamina" == eventType)
			m_iMaxStamina = *static_cast<int*>(data);
		});

	Register_PullCallback([this](_wstring eventType, void* data) {
		if (L"CurrentMana" == eventType)
			m_iCurrentMana = *static_cast<int*>(data);
		else if (L"MaxMana" == eventType)
			m_iMaxMana = *static_cast<int*>(data);
		});

}

void CObserver_Player_Status::Free()
{
	__super::Free();

}
