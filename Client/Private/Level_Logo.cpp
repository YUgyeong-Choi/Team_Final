#include "Level_Logo.h"

#include "GameInstance.h"
#include "Level_Loading.h"
#include "Static_UI.h"
#include "Dynamic_UI.h"
#include "UI_Video.h"
#include "UI_Button.h"
#include "UI_Text.h"
#include "UI_Feature_UV.h"

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
	if (!m_isReady)
	{
		for (int vk = 0; vk <= 254; ++vk) 
		{
			_int keyState = m_pGameInstance->Get_DIKeyState(vk);
			if (keyState & 0x8000)
			{
				m_isReady = true;
				Ready_Menu();
				return;
			}
				
		}
	}
	else
	{

		Check_Button();


		if (m_pGameInstance->Key_Down(DIK_F1))
		{
			m_eNextLevel = LEVEL::DH;
			if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, m_eNextLevel))))
				return;
		}

		if (m_pGameInstance->Key_Down(DIK_F2))
		{
			m_eNextLevel = LEVEL::JW;
			if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, m_eNextLevel))))
				return;
		}

		if (m_pGameInstance->Key_Down(DIK_F3))
		{
			m_eNextLevel = LEVEL::GL;
			if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, m_eNextLevel))))
				return;
		}

		if (m_pGameInstance->Key_Down(DIK_F4))
		{
			m_eNextLevel = LEVEL::YW;
			if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, m_eNextLevel))))
				return;
		}

		if (m_pGameInstance->Key_Down(DIK_F5))
		{
			m_eNextLevel = LEVEL::CY;
			if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, m_eNextLevel))))
				return;
		}

		if (m_pGameInstance->Key_Down(DIK_F6))
		{
			m_eNextLevel = LEVEL::YG;
			if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, m_eNextLevel))))
				return;
		}

		if (m_pGameInstance->Key_Down(DIK_F7))
		{
			m_eNextLevel = LEVEL::KRAT_CENTERAL_STATION;
			if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, m_eNextLevel))))
				return;
		}

		if (LEVEL::END != m_eNextLevel)
		{
			m_fDelay -= fTimeDelta;
			if (m_fDelay < 0.f)
			{
				if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, m_eNextLevel))))
					return;
			}

		}

		if (m_pGameInstance->Key_Down(DIK_F8))
		{
			if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::KRAT_HOTEL))))
				return;
		}
	}
	

}

HRESULT CLevel_Logo::Render()
{
	_wstring text = L"F1 : 동하, F2 : 장원, F3 : 경래, F4 : 영웅, F5 : 채영, F6 : 유경 , F7 : 클라크 스테이션 , F8 : 클라크 호텔";

	SetWindowText(g_hWnd, TEXT("로고레벨입니다."));

	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 860.f), XMVectorSet(1.f, 0.f, 0.f, 1.f));

	text = L"F5 : 렌더타겟 OnOff";
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 830.f), XMVectorSet(1.f, 0.f, 0.f, 1.f));

	text = L"F6 : 콜라이더 렌더 OnOff";
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 800.f), XMVectorSet(1.f, 0.f, 0.f, 1.f));

	if(!m_isReady)
	{
		text = L"아무 버튼이나 누르세요.";
		m_pGameInstance->Draw_Font_Centered(TEXT("Font_Medium"), text.c_str(), _float2(g_iWinSizeX * 0.2f, g_iWinSizeY * 0.5f), XMVectorSet(1.f, 1.f, 1.f, 1.f));
	}
	

	return S_OK;
}

HRESULT CLevel_Logo::Ready_Video()
{
	CStatic_UI::STATIC_UI_DESC eBackDesc = {};
	eBackDesc.fOffset = 0.1f;
	eBackDesc.fSpeedPerSec = 1.f;
	eBackDesc.fX = g_iWinSizeX * 0.5f;
	eBackDesc.fY = g_iWinSizeY * 0.5f;
	eBackDesc.fSizeX = g_iWinSizeX;
	eBackDesc.fSizeY = g_iWinSizeY;
	eBackDesc.iPassIndex = 0;
	eBackDesc.iTextureIndex = 0;
	eBackDesc.strTextureTag = TEXT("Prototype_Component_Texture_BackGround_Loading_Desk");
	eBackDesc.vColor = { 0.f,0.f,0.f,1.f };

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Static_UI"),
		static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background"), &eBackDesc)))
		return E_FAIL;


	CUI_Video::VIDEO_UI_DESC eDesc = {};
	eDesc.fOffset = 0.05f;
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

HRESULT CLevel_Logo::Ready_Menu()
{
	_float2 vRange = m_pGameInstance->Calc_Draw_Range(TEXT("Font_Medium"), TEXT("게임 시작"));

	CUI_Button::BUTTON_UI_DESC eButtonDesc = {};
	eButtonDesc.strTextureTag = L"";
	eButtonDesc.fOffset = 0.01f;
	eButtonDesc.fX = g_iWinSizeX * 0.225f;
	eButtonDesc.iTextureIndex = 0;
	eButtonDesc.vColor = { 0.8f,0.f,0.f,0.8f };
	eButtonDesc.fPadding = { 40.f, 15.f };
	eButtonDesc.fFontSize = 0.75f;
	eButtonDesc.fSizeX = vRange.x * eButtonDesc.fFontSize + eButtonDesc.fPadding.x;
	eButtonDesc.fSizeY = vRange.y * eButtonDesc.fFontSize + eButtonDesc.fPadding.y;

	

	eButtonDesc.fY = g_iWinSizeY * 0.35f;
	eButtonDesc.strCaption = TEXT("새 게임");

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Button"),
		static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button"), &eButtonDesc)))
		return E_FAIL;

	m_pButtons.push_back(static_cast<CUI_Button*>(m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button"))));

	Safe_AddRef(m_pButtons.back());

	eButtonDesc.fY = g_iWinSizeY * 0.45f;
	eButtonDesc.strCaption = TEXT("환경 설정");
	

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Button"),
		static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button"), &eButtonDesc)))
		return E_FAIL;

	m_pButtons.push_back(static_cast<CUI_Button*>(m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button"))));

	Safe_AddRef(m_pButtons.back());

	eButtonDesc.fY = g_iWinSizeY * 0.55f;
	eButtonDesc.strCaption = TEXT("팀원 소개");
	

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Button"),
		static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button"), &eButtonDesc)))
		return E_FAIL;

	m_pButtons.push_back(static_cast<CUI_Button*>(m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button"))));

	Safe_AddRef(m_pButtons.back());

	eButtonDesc.fY = g_iWinSizeY * 0.65f;
	eButtonDesc.strCaption = TEXT("게임 종료");

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Button"),
		static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button"), &eButtonDesc)))
		return E_FAIL;


	m_pButtons.push_back(static_cast<CUI_Button*>(m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button"))));

	Safe_AddRef(m_pButtons.back());

	m_pButtons[0]->Set_isHighlight(true);

	CDynamic_UI::DYNAMIC_UI_DESC eSelectDesc = {};
	eSelectDesc.fOffset = 0.01f;
	eSelectDesc.fDuration = 0.033f;
	eSelectDesc.fSizeX = g_iWinSizeX * 0.02f;
	eSelectDesc.fSizeY = g_iWinSizeY * 0.04f;
	eSelectDesc.fX = g_iWinSizeX * 0.175f;
	eSelectDesc.fY = g_iWinSizeY * 0.25f;
	eSelectDesc.iPassIndex = D_UI_SPRITE;
	eSelectDesc.iTextureIndex = 2;
	eSelectDesc.vColor = { 1.f,1.f,1.f,1.f };
	eSelectDesc.strTextureTag = TEXT("Prototype_Component_Texture_Button_Select");
	eSelectDesc.fAlpha = 0.5f;
	

	UI_FEATURE_UV_DESC *uvDesc = new UI_FEATURE_UV_DESC;
	uvDesc->iStartFrame = 0;
	uvDesc->iEndFrame = 16;
	uvDesc->fStartUV = { 0.f,0.f };
	uvDesc->fOffsetUV = { 0.25f,0.25f };
	uvDesc->isLoop = true;
	uvDesc->strProtoTag = ("Prototype_Component_UI_Feature_UV");

	eSelectDesc.FeatureDescs.push_back(uvDesc);

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Dynamic_UI"),
		static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button_Select"), &eSelectDesc)))
		return E_FAIL;

	Safe_Delete(uvDesc);

	m_pSelectUI = static_cast<CDynamic_UI*>(m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button_Select")));

	Safe_AddRef(m_pSelectUI);


	return S_OK;
}

void CLevel_Logo::Check_Button()
{
	
	if (m_pGameInstance->Key_Down(DIK_S))
	{
		if (m_iButtonIndex < m_pButtons.size() - 1)
		{
			m_pButtons[m_iButtonIndex]->Set_isHighlight(false);
			++m_iButtonIndex;
			m_pButtons[m_iButtonIndex]->Set_isHighlight(true);
		}
			
	}
	else if (m_pGameInstance->Key_Down(DIK_W))
	{
		if (m_iButtonIndex > 0)
		{
			m_pButtons[m_iButtonIndex]->Set_isHighlight(false);
			--m_iButtonIndex;
			m_pButtons[m_iButtonIndex]->Set_isHighlight(true);
			
		}
			
	}

	for (int i = 0; i < m_pButtons.size(); ++i)
	{
		_bool isHover = m_pButtons[i]->Check_MouseHover();

		if (isHover)
		{
			m_pButtons[m_iButtonIndex]->Set_isHighlight(false);
			m_iButtonIndex = i;
			m_pButtons[m_iButtonIndex]->Set_isHighlight(true);

			if (m_pGameInstance->Mouse_Down(DIM::LBUTTON))
			{
				Interation_Button(m_iButtonIndex);
			}

			break;
		}

	}

	if (m_pGameInstance->Key_Down(DIK_SPACE))
	{
		Interation_Button(m_iButtonIndex);
	}


	m_pButtons[m_iButtonIndex]->Set_isMouseHover(true);
	m_pSelectUI->Set_Position(g_iWinSizeX * 0.175f, g_iWinSizeY * (0.35f + 0.1f * m_iButtonIndex));
}

void CLevel_Logo::Interation_Button(_int& iIndex)
{
	switch (iIndex)
	{
	case 0:
		m_eNextLevel = LEVEL::KRAT_CENTERAL_STATION;
		break;
	case 1:
		break;
	case 2:
		break;
	case 3:
		PostQuitMessage(0);
		break;
	default:
		break;
	}
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
	Safe_Release(m_pSelectUI);

	for(auto& pButton: m_pButtons)
		Safe_Release(pButton);
}
