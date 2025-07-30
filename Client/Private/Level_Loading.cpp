#include "Level_Loading.h"


#include "Level_KratCentralStation.h"
#include "Level_KratHotel.h"
#include "Level_Logo.h"
#include "Loader.h"

#include "Level_DH.h"
#include "Level_JW.h"
#include "Level_YW.h"
#include "Level_GL.h"
#include "Level_CY.h"
#include "Level_YG.h"

#include "GameInstance.h"

#include "Static_UI.h"
#include "Dynamic_UI.h"
#include "UI_Video.h"
#include "UI_Text.h"

CLevel_Loading::CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
{

}

HRESULT CLevel_Loading::Initialize(LEVEL eNextLevelID)
{
	m_eNextLevelID = eNextLevelID;


	/* 로딩레벨 자체에 필요한 객체를 생성한다. */
	/* 배경, 로딩바, 버튼, font */

	/* 로딩의 역할(다음레벨에 필요한 자원(Resource)(텍스쳐, 모델, 사운드 등등등 )을 생성하는)을 
	수행할 로더객체를 생성한다. */
	m_pLoader = CLoader::Create(m_pDevice, m_pContext, m_eNextLevelID, ref(m_fRatio));
	if (nullptr == m_pLoader)
		return E_FAIL;

	if (m_eNextLevelID != LEVEL::LOGO)
	{
		if (FAILED(Ready_Loading()))
			return E_FAIL;

	}

	
	return S_OK;
}

void CLevel_Loading::Update(_float fTimeDelta)
{


	if (m_pGameInstance->Key_Down(DIK_SPACE))
	{
		if (true == m_pLoader->isFinished())
		{
			CLevel* pLevel = { nullptr };


			switch (m_eNextLevelID)
			{
			case LEVEL::LOGO:
				pLevel = CLevel_Logo::Create(m_pDevice, m_pContext);
				break;
			case LEVEL::KRAT_CENTERAL_STATION:
				pLevel = CLevel_KratCentralStation::Create(m_pDevice, m_pContext);
				break;
			case LEVEL::KRAT_HOTEL:
				pLevel = CLevel_KratHotel::Create(m_pDevice, m_pContext);
				break;
			case LEVEL::DH:
				pLevel = CLevel_DH::Create(m_pDevice, m_pContext);
				break;
			case LEVEL::JW:
				pLevel = CLevel_JW::Create(m_pDevice, m_pContext);
				break;
			case LEVEL::GL:
				pLevel = CLevel_GL::Create(m_pDevice, m_pContext);
				break;
			case LEVEL::YW:
				pLevel = CLevel_YW::Create(m_pDevice, m_pContext);
				break;
			case LEVEL::CY:
				pLevel = CLevel_CY::Create(m_pDevice, m_pContext);
				break;
			case LEVEL::YG:
				pLevel = CLevel_YG::Create(m_pDevice, m_pContext);
				break;
			}

			if (nullptr == pLevel)
				return;

			if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(m_eNextLevelID), pLevel)))
				return;
							
		}
	}	



	if (m_eNextLevelID == LEVEL::LOGO)
	{
		if (true == m_pLoader->isFinished())
		{
			CLevel* pLevel = { nullptr };

			pLevel = CLevel_Logo::Create(m_pDevice, m_pContext);

			if (nullptr == pLevel)
				return;

			if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(m_eNextLevelID), pLevel)))
				return;

		}
	}
}

HRESULT CLevel_Loading::Render()
{
	m_pLoader->Output_LoadingText();

	_wstring text = L"하나둘삼넷오여섯칠팔아홉공 \n Test Test 중";
	m_pGameInstance->Draw_Font(TEXT("Font_Medium"), text.c_str(), _float2(g_iWinSizeX * 0.25f, g_iWinSizeY * 0.725f), XMVectorSet(0.f, 0.f, 0.f, 1.f), 0.f, _float2(0.f, 0.f), g_iWinSizeY / 1350.f);

	
	_int iPercent = int(floorf(m_fRatio * 100.f));
	
	_wstring loadingText = L"거짓말하는 중 ...   " + to_wstring(iPercent) + L" %";
	m_pGameInstance->Draw_Font(TEXT("Font_Bold"), loadingText.c_str(), _float2(g_iWinSizeX * 0.77f, g_iWinSizeY * 0.95f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f, _float2(0.f, 0.f), g_iWinSizeY / 1200.f);

	return S_OK;
}

HRESULT CLevel_Loading::Ready_Loading()
{
	json j;

	ifstream file("../Bin/Save/UI/Loading.json");

	file >> j;

	for (const auto& eUIJson : j)
	{
		string protoTag = eUIJson["ProtoTag"];

		if ("Prototype_GameObject_Static_UI" == protoTag)
		{

			(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), StringToWStringU8(protoTag), ENUM_CLASS(LEVEL::LOADING), TEXT("Layer_Background_Static"), nullptr));

			CStatic_UI* pObj = static_cast<CStatic_UI*> (m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::LOADING), TEXT("Layer_Background_Static")));

			pObj->Deserialize(eUIJson);

			pObj->Update_Data();

			static_cast<CUIObject*>(m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::LOADING), TEXT("Layer_Background_Static")))->Set_isVignetting(true);

		}
		else if ("Prototype_GameObject_Dynamic_UI" == protoTag)
		{

			(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), StringToWStringU8(protoTag), ENUM_CLASS(LEVEL::LOADING), TEXT("Layer_Background_Dynamic"), nullptr));

			CDynamic_UI* pObj = static_cast<CDynamic_UI*> (m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::LOADING), TEXT("Layer_Background_Dynamic")));

			pObj->Deserialize(eUIJson);

			pObj->Update_Data();

		

		}
		else if ("Prototype_GameObject_UI_Text" == protoTag)
		{
			
			(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), StringToWStringU8(protoTag), ENUM_CLASS(LEVEL::LOADING), TEXT("Layer_Background_Text"), nullptr));

			CUI_Text* pObj = static_cast<CUI_Text*> (m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::LOADING), TEXT("Layer_Background_Text")));

			pObj->Deserialize(eUIJson);

			pObj->Update_Data();

			
		}

	}
	file.close();

	
	static_cast<CUIObject*>(m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::LOADING), TEXT("Layer_Background_Static")))->Set_isVignetting(false);

	


	return S_OK;
}



CLevel_Loading* CLevel_Loading::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID)
{
	CLevel_Loading* pInstance = new CLevel_Loading(pDevice, pContext);

	if (FAILED(pInstance->Initialize(eNextLevelID)))
	{
		MSG_BOX("Failed to Created : CLevel_Loading");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_Loading::Free()
{
	__super::Free();

	Safe_Release(m_pLoader);
}
