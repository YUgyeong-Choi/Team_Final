#include "Level_Logo.h"

#include "GameInstance.h"
#include "Level_Loading.h"
#include "Static_UI.h"
#include "Dynamic_UI.h"
#include "UI_Video.h"
#include "UI_Button.h"
#include "UI_Text.h"
#include "UI_Feature_UV.h"
#include "UI_Container.h"
#include "UI_Guide.h"
#include "UI_Manager.h"

CLevel_Logo::CLevel_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
{

}

HRESULT CLevel_Logo::Initialize()
{

	m_pGameInstance->SetCurrentLevelIndex(ENUM_CLASS(LEVEL::LOGO));

	if (FAILED(Ready_Video()))
		return E_FAIL;

	if (FAILED(Ready_Guide()))
		return E_FAIL;

	/* [ 사운드 ] */
	m_pBGM = m_pGameInstance->Get_Single_Sound("MU_MS_Title_Piano_03");
	m_pBGM->Set_Volume(0.5f * g_fBGMSoundVolume);
	m_pBGM->Play();
	return S_OK;
}

void CLevel_Logo::Update(_float fTimeDelta)
{	


	if (!m_isReady)
	{
		for (int vk = 0; vk <= 254; ++vk) 
		{
			// 아무 키나 먹게
			_int keyState = m_pGameInstance->Get_DIKeyState(vk);
			if (keyState & 0x8000)
			{
				m_isReady = true;
				CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Selected_Default_03");
				Ready_Menu();
				return;
			}
				
		}
	}
	else
	{
		if (m_isOpen)
		{

			if (2 == m_iButtonIndex)
			{
				if (!m_pGuideBack->Get_isActive())
				{
					m_pTeammate->Active_Update(true);
					m_pGuideBack->Set_isActive(true);
				}


				if (m_pGameInstance->Key_Down(DIK_SPACE))
				{
					m_pTeammate->Active_Update(false);
					m_pGuideBack->Set_isActive(false);

					m_isOpen = false;
					return;
				}
			}
		

			

			
			return;
		}
		

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
	SetWindowText(g_hWnd, TEXT("Lies Of P"));

	if(!m_isReady)
	{
		_wstring text = L"아무 버튼이나 누르세요.";
		m_pGameInstance->Draw_Font_Centered(TEXT("Font_Medium"), text.c_str(), _float2(g_iWinSizeX * 0.2f, g_iWinSizeY * 0.5f), XMVectorSet(1.f, 1.f, 1.f, 1.f));
	}
	

	return S_OK;
}

HRESULT CLevel_Logo::Ready_Video()
{
	CStatic_UI::STATIC_UI_DESC eBackDesc = {};
	eBackDesc.fOffset = 0.7f;
	eBackDesc.fSpeedPerSec = 1.f;
	eBackDesc.fX = g_iWinSizeX * 0.5f;
	eBackDesc.fY = g_iWinSizeY * 0.5f;
	eBackDesc.fSizeX = g_iWinSizeX;
	eBackDesc.fSizeY = g_iWinSizeY;
	eBackDesc.iPassIndex = 0;
	eBackDesc.iTextureIndex = 0;
	eBackDesc.strTextureTag = TEXT("Prototype_Component_Texture_Button_Select");
	eBackDesc.vColor = { 0.f,0.f,0.f,1.f };

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Static_UI"),
		static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background"), &eBackDesc)))
		return E_FAIL;


	CUI_Video::VIDEO_UI_DESC eDesc = {};
	eDesc.eType = CUI_Video::VIDEO_TYPE::LOGO;
	eDesc.fOffset = 0.5f;
	eDesc.fInterval = 0.05f;
	eDesc.fSpeedPerSec = 1.f;
	eDesc.strVideoPath = TEXT("../Bin/Resources/Video/Title.mp4");
	eDesc.fX = g_iWinSizeX * 0.5f;
	eDesc.fY = g_iWinSizeY * 0.5f;
	eDesc.fSizeX = g_iWinSizeX;
	eDesc.fSizeY = g_iWinSizeY;
	eDesc.fAlpha = 0.f;

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
	eButtonDesc.fOffset = 0.45f;
	eButtonDesc.fX = g_iWinSizeX * 0.225f;
	eButtonDesc.iTextureIndex = 0;
	eButtonDesc.vColor = { 0.8f,0.f,0.f,0.8f };
	eButtonDesc.fPadding = {g_iWinSizeX * -0.0075f, 0.f };
	eButtonDesc.fFontSize = 0.75f;
	eButtonDesc.fSizeX = vRange.x * eButtonDesc.fFontSize ;
	eButtonDesc.fSizeY = vRange.y * eButtonDesc.fFontSize ;

	

	eButtonDesc.fY = g_iWinSizeY * 0.35f;
	eButtonDesc.strCaption = TEXT("새 게임");

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Button"),
		static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button"), &eButtonDesc)))
		return E_FAIL;

	m_pButtons.push_back(static_cast<CUI_Button*>(m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button"))));


	eButtonDesc.fY = g_iWinSizeY * 0.45f;
	eButtonDesc.strCaption = TEXT("환경 설정");
	

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Button"),
		static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button"), &eButtonDesc)))
		return E_FAIL;

	m_pButtons.push_back(static_cast<CUI_Button*>(m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button"))));

	eButtonDesc.fY = g_iWinSizeY * 0.55f;
	eButtonDesc.strCaption = TEXT("팀원 소개");
	

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Button"),
		static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button"), &eButtonDesc)))
		return E_FAIL;

	m_pButtons.push_back(static_cast<CUI_Button*>(m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button"))));

	

	eButtonDesc.fY = g_iWinSizeY * 0.65f;
	eButtonDesc.strCaption = TEXT("게임 종료");

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Button"),
		static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button"), &eButtonDesc)))
		return E_FAIL;


	m_pButtons.push_back(static_cast<CUI_Button*>(m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button"))));

	for (auto& pObj : m_pButtons)
		Safe_AddRef(pObj);

	m_pButtons[0]->Set_isHighlight(true);

	CDynamic_UI::DYNAMIC_UI_DESC eSelectDesc = {};
	eSelectDesc.fOffset = 0.01f;
	eSelectDesc.fDuration = 0.033f;
	eSelectDesc.fSizeX = g_iWinSizeX * 0.02f;
	eSelectDesc.fSizeY = g_iWinSizeY * 0.04f;
	eSelectDesc.fX = g_iWinSizeX * 0.175f;
	eSelectDesc.fY = g_iWinSizeY * 0.25f;
	eSelectDesc.iPassIndex = D_UI_SPRITE;
	eSelectDesc.iTextureIndex = 0;
	eSelectDesc.vColor = { 1.f,1.f,1.f,1.f };
	eSelectDesc.strTextureTag = TEXT("Prototype_Component_Texture_Button_Select");
	eSelectDesc.fAlpha = 0.5f;


	UI_FEATURE_UV_DESC uvDesc = {};
	uvDesc.iStartFrame = 0;
	uvDesc.iEndFrame = 16;
	uvDesc.fStartUV = { 0.f,0.f };
	uvDesc.fOffsetUV = { 0.25f,0.25f };
	uvDesc.isLoop = true;
	uvDesc.strProtoTag = ("Prototype_Component_UI_Feature_UV");

	eSelectDesc.FeatureDescs.push_back(&uvDesc);

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Dynamic_UI"),
		static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button_Select"), &eSelectDesc)))
		return E_FAIL;

	
	m_pSelectUI = dynamic_cast<CDynamic_UI*>(m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Background_Button_Select")));

	if (!m_pSelectUI)
		return E_FAIL;
 	


	return S_OK;
}

HRESULT CLevel_Logo::Ready_Guide()
{
	CUI_Container::UI_CONTAINER_DESC eDesc = {};

	eDesc.strFilePath = TEXT("../Bin/Save/UI/TeamMate.json");

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"),
		static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Guide"), &eDesc)))
		return E_FAIL;

	m_pTeammate = static_cast<CUI_Container*>(m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::LOGO), TEXT("Layer_Guide")));

	CStatic_UI::STATIC_UI_DESC eBackDesc = {};
	eBackDesc.fOffset = 0.4f;
	eBackDesc.fSpeedPerSec = 1.f;
	eBackDesc.fX = g_iWinSizeX * 0.5f;
	eBackDesc.fY = g_iWinSizeY * 0.5f;
	eBackDesc.fSizeX = g_iWinSizeX;
	eBackDesc.fSizeY = g_iWinSizeY;
	eBackDesc.iPassIndex = UI_BLEND;
	eBackDesc.iTextureIndex = 0;
	eBackDesc.strTextureTag = TEXT("Prototype_Component_Texture_BackGround_Loading_Desk");
	eBackDesc.vColor = { 0.f,0.f,0.f,0.5f };

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Static_UI"),
		static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Guide"), &eBackDesc)))
		return E_FAIL;

	



	m_pGuideBack = (m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::LOGO), TEXT("Layer_Guide")));

	m_pTeammate->Active_Update(false);
	m_pGuideBack->Set_isActive(false);

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
			CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Hovered_Default_02");
		}
			
	}
	else if (m_pGameInstance->Key_Down(DIK_W))
	{
		if (m_iButtonIndex > 0)
		{
			m_pButtons[m_iButtonIndex]->Set_isHighlight(false);
			--m_iButtonIndex;
			m_pButtons[m_iButtonIndex]->Set_isHighlight(true);
			CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Hovered_Default_02");
		}
			
	}

	_bool isPlayMouseHoverSound = false;

	for (int i = 0; i < m_pButtons.size(); ++i)
	{
		_bool isHover = m_pButtons[i]->Check_MouseHover();

		if (isHover)
		{
			m_pButtons[m_iButtonIndex]->Set_isHighlight(false);
			if (i != m_iButtonIndex)
				 CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Hovered_Default_02");
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
	if(nullptr != m_pSelectUI)
		m_pSelectUI->Set_Position(g_iWinSizeX * 0.175f, g_iWinSizeY * (0.35f + 0.1f * m_iButtonIndex));
}

void CLevel_Logo::Interation_Button(_int& iIndex)
{
	switch (iIndex)
	{
	case 0:
		m_eNextLevel = LEVEL::KRAT_CENTERAL_STATION;
		CUI_Manager::Get_Instance()->Sound_Play("SE_UI_StartGame_03");
		break;
	case 1:
		//m_pGuide->Active_Update(true);
		//m_isOpen = true;
		break;
	case 2:
		m_isOpen = true;
		CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Selected_Default_03");
		break;
	case 3:
		PostQuitMessage(0);
		CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Selected_Default_03");
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

	if (m_pBGM)
	{
		m_pBGM->Stop();
		Safe_Release(m_pBGM);
	}

	Safe_Release(m_pMainUI);

	for(auto& pButton: m_pButtons)
		Safe_Release(pButton);
	m_pButtons.clear();

	
}
