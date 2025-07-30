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

		CUI_Text::TEXT_UI_DESC eTextDesc = {};
		eTextDesc.fOffset = { 0.0f };
		eTextDesc.fFontScale = { 1.f };
		eTextDesc.fFontOffset = { 0.f,0.f };
		eTextDesc.fSizeX = 0.f;
		eTextDesc.fSizeY = 0.f;
		eTextDesc.fX = g_iWinSizeX * 0.2f;
		eTextDesc.fY = g_iWinSizeY * 0.2f;
		eTextDesc.isCenter = true;
		eTextDesc.strCaption = TEXT("Test");
		eTextDesc.strFontTag = TEXT("Font_Medium");
		eTextDesc.vColor = { 1.f,1.f,1.f,1.f };

		if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Text"),
			static_cast<_uint>(LEVEL::LOADING), TEXT("Layer_Background_Text"), &eTextDesc)))
			return E_FAIL;

		static_cast<CUIObject*>(m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::LOADING), TEXT("Layer_Background_Text")))->FadeStart(1.f, 0.f, 1.5f);
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

	{
		json j;

		ifstream file("../Bin/DataFiles/UI/Loading_Static.json");

		file >> j;

		for (const auto& eUIJson : j)
		{
			CStatic_UI::STATIC_UI_DESC eStaticDesc = {};

			eStaticDesc.fOffset = eUIJson["Offset"];
			eStaticDesc.iPassIndex = eUIJson["PassIndex"];
			eStaticDesc.iTextureIndex = eUIJson["TextureIndex"];
			eStaticDesc.fSizeX = eUIJson["fSizeX"].get<float>() * g_iWinSizeX;
			eStaticDesc.fSizeY = eUIJson["fSizeY"].get<float>() * g_iWinSizeY;
			eStaticDesc.fX = eUIJson["fX"].get<float>() * g_iWinSizeX;
			eStaticDesc.fY = eUIJson["fY"].get<float>() * g_iWinSizeY;

			eStaticDesc.vColor = { eUIJson["Color"][0], eUIJson["Color"][1], eUIJson["Color"][2],eUIJson["Color"][3], };

			string textureTag = eUIJson["TextureTag"];
			eStaticDesc.strTextureTag = StringToWString(textureTag);

			if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Static_UI"),
				static_cast<_uint>(LEVEL::LOADING), TEXT("Layer_Background_Static"), &eStaticDesc)))
				return E_FAIL;

			static_cast<CUIObject*>(m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::LOADING), TEXT("Layer_Background_Static")))->Set_isVignetting(true);
		}

		static_cast<CUIObject*>(m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::LOADING), TEXT("Layer_Background_Static")))->Set_isVignetting(false);
		file.close();
	}

	

	{

		json j;

		ifstream file("../Bin/DataFiles/UI/Loading_Dynamic.json");

		file >> j;

		for (const auto& eUIJson : j)
		{
			CDynamic_UI::DYNAMIC_UI_DESC eDesc = {};


			eDesc.fOffset = eUIJson["Offset"];
			eDesc.iPassIndex = eUIJson["PassIndex"];
			eDesc.iTextureIndex = eUIJson["TextureIndex"];
			eDesc.fSizeX = eUIJson["fSizeX"].get<float>() * g_iWinSizeX;
			eDesc.fSizeY = eUIJson["fSizeY"].get<float>() * g_iWinSizeY;
			eDesc.fX = eUIJson["fX"].get<float>() * g_iWinSizeX;
			eDesc.fY = eUIJson["fY"].get<float>() * g_iWinSizeY;
			eDesc.fDuration = eUIJson["Duration"].get<float>();
			eDesc.vColor = { eUIJson["Color"][0], eUIJson["Color"][1], eUIJson["Color"][2],eUIJson["Color"][3] };

			string textureTag = eUIJson["TextureTag"];
			eDesc.strTextureTag = wstring(textureTag.begin(), textureTag.end());
	
			// 기능 읽어서 각 프로토타입에 맞게 구조체 생성해서 넣자
			if (!eUIJson.contains("Features"))
				continue;

			const auto& features = eUIJson["Features"];

			for (const auto& feature : features)
			{
				std::string protoTag = feature["ProtoTag"];

				if (protoTag.find("UV") != protoTag.npos)
				{
					UI_FEATURE_UV_DESC* efeatureDesc = new UI_FEATURE_UV_DESC;
					efeatureDesc->strProtoTag = (protoTag);
					efeatureDesc->iStartFrame = feature["iStartFrame"];
					efeatureDesc->iEndFrame = feature["iEndFrame"];
					efeatureDesc->isLoop = feature["isLoop"];

					efeatureDesc->fStartUV = { feature["fStartUV"]["x"],  feature["fStartUV"]["y"] };
					efeatureDesc->fOffsetUV = { feature["fOffsetUV"]["x"],  feature["fOffsetUV"]["y"] };

					eDesc.FeatureDescs.push_back(efeatureDesc);
				}
				else if (protoTag.find("Pos") != protoTag.npos)
				{
					UI_FEATURE_POS_DESC* efeatureDesc = new UI_FEATURE_POS_DESC;
					efeatureDesc->strProtoTag = (protoTag);
					efeatureDesc->iStartFrame = feature["iStartFrame"];
					efeatureDesc->iEndFrame = feature["iEndFrame"];
					efeatureDesc->isLoop = feature["isLoop"];

					efeatureDesc->fStartPos = { feature["fStartPos"]["x"],  feature["fStartPos"]["y"] };
					efeatureDesc->fEndPos = { feature["fEndPos"]["x"],  feature["fEndPos"]["y"] };

					eDesc.FeatureDescs.push_back(efeatureDesc);
				}
				else if (protoTag.find("Scale") != protoTag.npos)
				{
					UI_FEATURE_SCALE_DESC* efeatureDesc = new UI_FEATURE_SCALE_DESC;
					efeatureDesc->strProtoTag = (protoTag);
					efeatureDesc->iStartFrame = feature["iStartFrame"];
					efeatureDesc->iEndFrame = feature["iEndFrame"];
					efeatureDesc->isLoop = feature["isLoop"];

					efeatureDesc->fStartScale = { feature["fStartScale"]["x"],  feature["fStartScale"]["y"] };
					efeatureDesc->fEndScale = { feature["fEndScale"]["x"],  feature["fEndScale"]["y"] };

					eDesc.FeatureDescs.push_back(efeatureDesc);
				}
				else if (protoTag.find("Fade") != protoTag.npos)
				{
					UI_FEATURE_FADE_DESC* efeatureDesc = new UI_FEATURE_FADE_DESC;
					efeatureDesc->strProtoTag = (protoTag);
					efeatureDesc->iStartFrame = feature["iStartFrame"];
					efeatureDesc->iEndFrame = feature["iEndFrame"];
					efeatureDesc->isLoop = feature["isLoop"];

					efeatureDesc->fStartAlpha = feature["StartAlpha"];
					efeatureDesc->fEndAlpha = feature["fEndAlpha"];

					eDesc.FeatureDescs.push_back(efeatureDesc);
				}
			}


			if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Dynamic_UI"),
				static_cast<_uint>(LEVEL::LOADING), TEXT("Layer_Background_Dynamic"), &eDesc)))
				return E_FAIL;

			for (auto& pDesc : eDesc.FeatureDescs)
			{
				Safe_Delete(pDesc);
			}

		}

		file.close();
	}
	


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
