#include "Level_Loading.h"


#include "Level_KratCentralStation.h"
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
	m_pLoader = CLoader::Create(m_pDevice, m_pContext, m_eNextLevelID);
	if (nullptr == m_pLoader)
		return E_FAIL;

	if (FAILED(Ready_Loading()))
		return E_FAIL;
	
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
}

HRESULT CLevel_Loading::Render()
{
	m_pLoader->Output_LoadingText();

	return S_OK;
}

HRESULT CLevel_Loading::Ready_Loading()
{
	CStatic_UI::STATIC_UI_DESC eUIDesc = {};

	eUIDesc.fX = g_iWinSizeX * 0.5f;
	eUIDesc.fY = g_iWinSizeY * 0.5f;
	eUIDesc.fSizeX = g_iWinSizeX;
	eUIDesc.fSizeY = g_iWinSizeY;
	eUIDesc.iPassIndex = UI_DEFAULT;
	eUIDesc.strTextureTag = TEXT("Prototype_Component_Texture_BackGround_Loading_Desk");

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Static_UI"),
		static_cast<_uint>(LEVEL::LOADING), TEXT("Layer_Background_Static"), &eUIDesc)))
		return E_FAIL;

	eUIDesc.fSizeX = g_iWinSizeX * 1.2f;
	eUIDesc.fSizeY = g_iWinSizeY * 1.1f;
	eUIDesc.iPassIndex = UI_DISCARD_ALAPH;
	eUIDesc.strTextureTag = TEXT("Prototype_Component_Texture_BackGround_Loading_Paper");

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Static_UI"),
		static_cast<_uint>(LEVEL::LOADING), TEXT("Layer_Background_Static"), &eUIDesc)))
		return E_FAIL;


	eUIDesc.fX = g_iWinSizeX * 0.5f;
	eUIDesc.fY = g_iWinSizeY * 0.5f;
	eUIDesc.fSizeX = g_iWinSizeX;
	eUIDesc.fSizeY = g_iWinSizeY;
	eUIDesc.iPassIndex = UI_DISCARD_DARK;
	eUIDesc.strTextureTag = TEXT("Prototype_Component_Texture_BackGround_Loading_Photo");

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Static_UI"),
		static_cast<_uint>(LEVEL::LOADING), TEXT("Layer_Background_Static"), &eUIDesc)))
		return E_FAIL;

	eUIDesc.fX = g_iWinSizeX * 0.475f;
	eUIDesc.fY = g_iWinSizeY * 0.8f;
	eUIDesc.fSizeX = g_iWinSizeX * 0.6f;
	eUIDesc.fSizeY = g_iWinSizeY * 0.4f;
	eUIDesc.iPassIndex = UI_DISCARD_ALAPH;
	eUIDesc.strTextureTag = TEXT("Prototype_Component_Texture_BackGround_Loading_Tip");

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Static_UI"),
		static_cast<_uint>(LEVEL::LOADING), TEXT("Layer_Background_Static"), &eUIDesc)))
		return E_FAIL;

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
