#include "Level_Logo.h"

#include "GameInstance.h"
#include "Level_Loading.h"
#include "Static_UI.h"


CLevel_Logo::CLevel_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
{

}

HRESULT CLevel_Logo::Initialize()
{

	return S_OK;
}

void CLevel_Logo::Update(_float fTimeDelta)
{
	
	if(m_pGameInstance->Key_Down(DIK_F1))
	{
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::DH))))
			return;
	}

	if (m_pGameInstance->Key_Down(DIK_F2))
	{
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::JW))))
			return;
	}

	if (m_pGameInstance->Key_Down(DIK_F3))
	{
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::GL))))
			return;
	}

	if (m_pGameInstance->Key_Down(DIK_F4))
	{
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::YW))))
			return;
	}

	if (m_pGameInstance->Key_Down(DIK_F5))
	{
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::CY))))
			return;
	}

	if (m_pGameInstance->Key_Down(DIK_F6))
	{
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::YG))))
			return;
	}
}

HRESULT CLevel_Logo::Render()
{
	SetWindowText(g_hWnd, TEXT("로고레벨입니다."));
	_wstring text = L"F1 : 동하, F2 : 장원, F3 : 경래, F4 : 영웅, F5 : 채영, F6 : 유경";
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 680.f), XMVectorSet(1.f, 0.f, 0.f, 1.f));

	text = L"F11 : 렌더타겟 OnOff";
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 650.f), XMVectorSet(1.f, 0.f, 0.f, 1.f));

	text = L"F10 : 콜라이더 렌더 OnOff";
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 620.f), XMVectorSet(1.f, 0.f, 0.f, 1.f));

	return S_OK;
}


CLevel_Logo* CLevel_Logo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Logo* pInstance = new CLevel_Logo(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_Logo");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_Logo::Free()
{
	__super::Free();

}
