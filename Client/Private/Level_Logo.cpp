#include "Level_Logo.h"

#include "GameInstance.h"
#include "Level_Loading.h"
#include "Static_UI.h"
#include "UI_Video.h"


CLevel_Logo::CLevel_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
{

}

HRESULT CLevel_Logo::Initialize()
{
	if (FAILED(Ready_Video()))
		return E_FAIL;


	return S_OK;
}

void CLevel_Logo::Update(_float fTimeDelta)
{
	
	if(m_pGameInstance->Key_Down(DIK_F1))
	{
		m_eNextLevel = LEVEL::DH;
	}

	if (m_pGameInstance->Key_Down(DIK_F2))
	{
		m_eNextLevel = LEVEL::JW;
	}

	if (m_pGameInstance->Key_Down(DIK_F3))
	{
		m_eNextLevel = LEVEL::GL;
	}

	if (m_pGameInstance->Key_Down(DIK_F4))
	{
		m_eNextLevel = LEVEL::YW;
	}

	if (m_pGameInstance->Key_Down(DIK_F5))
	{
		m_eNextLevel = LEVEL::CY;
	}

	if (m_pGameInstance->Key_Down(DIK_F6))
	{
		m_eNextLevel = LEVEL::YG;
	}

	if (m_pGameInstance->Key_Down(DIK_F7))
	{
		m_eNextLevel = LEVEL::KRAT_CENTERAL_STATION;
	}

	if (LEVEL::END != m_eNextLevel)
	{
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, m_eNextLevel))))
			return;
	}

	if (m_pGameInstance->Key_Down(DIK_F8))
	{
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::KRAT_HOTEL))))
			return;
	}

}

HRESULT CLevel_Logo::Render()
{
	SetWindowText(g_hWnd, TEXT("로고레벨입니다."));
	_wstring text = L"F1 : 동하, F2 : 장원, F3 : 경래, F4 : 영웅, F5 : 채영, F6 : 유경 , F7 : 클라크 스테이션 , F8 : 클라크 호텔";
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 860.f), XMVectorSet(1.f, 0.f, 0.f, 1.f));

	text = L"F5 : 렌더타겟 OnOff";
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 830.f), XMVectorSet(1.f, 0.f, 0.f, 1.f));

	text = L"F6 : 콜라이더 렌더 OnOff";
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 800.f), XMVectorSet(1.f, 0.f, 0.f, 1.f));

	return S_OK;
}

HRESULT CLevel_Logo::Ready_Video()
{
	CUI_Video::VIDEO_UI_DESC eDesc = {};
	eDesc.fOffset = 0.1f;
	eDesc.fInterval = 0.05f;
	eDesc.fSpeedPerSec = 1.f;
	eDesc.strVideoPath = TEXT("../Bin/Resources/Video/Title.mp4");
	eDesc.fX = g_iWinSizeX * 0.5f;
	eDesc.fY = g_iWinSizeY * 0.5f;
	eDesc.fSizeX = g_iWinSizeX;
	eDesc.fSizeY = g_iWinSizeY;

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Video"),
		static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Video"), &eDesc)))
		return E_FAIL;


	m_pMainUI = static_cast<CUI_Video*>(m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Video")));

	Safe_AddRef(m_pMainUI);

	m_pMainUI->FadeStart(0.f, 1.f, 3.f);

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

	Safe_Release(m_pMainUI);
}
