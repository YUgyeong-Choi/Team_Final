#include "UI_SelectLocation.h"
#include "GameInstance.h"
#include "Stargazer.h"
#include "UI_Button.h"
#include "UI_Manager.h"

CUI_SelectLocation::CUI_SelectLocation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUI_Container{pDevice, pContext}
{
}

CUI_SelectLocation::CUI_SelectLocation(const CUI_SelectLocation& Prototype)
	:CUI_Container{Prototype}
{
}

HRESULT CUI_SelectLocation::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_SelectLocation::Initialize(void* pArg)
{
	UI_SELECT_LOCATION_DESC* pDesc = static_cast<UI_SELECT_LOCATION_DESC*>(pArg);

	m_pTarget = pDesc->pTarget;

	UI_CONTAINER_DESC eDesc = {};

	// 
	eDesc.strFilePath = TEXT("../Bin/Save/UI/SelectLocation/SelectLocation_Background.json");

	if (FAILED(__super::Initialize(&eDesc)))
		return E_FAIL;

	eDesc.strFilePath = TEXT("../Bin/Save/UI/SelectLocation/SelectLocation_LocationImg.json");

	m_pLocationImg = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

	eDesc.strFilePath = TEXT("../Bin/Save/UI/SelectLocation/SelectLocation_Button.json");

	m_pBehindButtons = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));


	Ready_Button();


	return S_OK;
}

void CUI_SelectLocation::Priority_Update(_float fTimeDelta)
{
	if (!m_bInit)
	{
		m_bInit = true;
		return;
	}

	if (m_pGameInstance->Key_Down(DIK_ESCAPE))
	{
		Set_bDead();
		for (auto& pButton : m_pButtons)
			pButton->Set_bDead();

		m_pSelectUI->Set_bDead();

		CStargazer* pStar = static_cast<CStargazer*>(m_pTarget);

		pStar->Script_Activate();



		return;

	}

	__super::Priority_Update(fTimeDelta);

	m_pLocationImg->Priority_Update(fTimeDelta);

	m_pBehindButtons->Priority_Update(fTimeDelta);

	for (auto& pButton : m_pButtons)
		pButton->Priority_Update(fTimeDelta);

	m_pSelectUI->Priority_Update(fTimeDelta);

	Update_Button();
}

void CUI_SelectLocation::Update(_float fTimeDelta)
{
	if (m_bDead)
		return;

	__super::Update(fTimeDelta);

	m_pLocationImg->Update(fTimeDelta);

	m_pBehindButtons->Update(fTimeDelta);

	for (auto& pButton : m_pButtons)
		pButton->Update(fTimeDelta);

	m_pSelectUI->Update(fTimeDelta);
}

void CUI_SelectLocation::Late_Update(_float fTimeDelta)
{
	if (m_bDead)
		return;

	__super::Late_Update(fTimeDelta);

	m_pLocationImg->Late_Update(fTimeDelta);

	m_pBehindButtons->Late_Update(fTimeDelta);

	for (auto& pButton : m_pButtons)
		pButton->Late_Update(fTimeDelta);

	m_pSelectUI->Late_Update(fTimeDelta);
}

HRESULT CUI_SelectLocation::Render()
{
	return S_OK;
}

HRESULT CUI_SelectLocation::Ready_Button()
{
	// 별바라기 레이어를 찾고
	// 몇개가 활성화 되있는지 찾고
	// 활성화 된거만 tag 받아와서
	// 그 개수 만큼 만든다

	_float2 vRange = {};

	CUI_Button::BUTTON_UI_DESC eButtonDesc = {};
	eButtonDesc.strTextureTag = L"";
	eButtonDesc.fOffset = 0.f;
	eButtonDesc.fX = g_iWinSizeX * 0.125f;
	eButtonDesc.iTextureIndex = 0;
	eButtonDesc.vColor = { 0.8f,0.f,0.f,0.8f };
	eButtonDesc.fPadding = {};
	eButtonDesc.fFontSize = 0.75f;




	eButtonDesc.fY = g_iWinSizeY * 0.3f;
	eButtonDesc.strCaption = {};

	list<CGameObject*>& ObjList = m_pGameInstance->Get_ObjectList(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Stargazer"));

	_float2 fSize = m_pGameInstance->Calc_Draw_Range(TEXT("Font_Medium"), TEXT("크라트 기차역 광장"));

	for (CGameObject* pObj : ObjList)
	{
		if (static_cast<CStargazer*>(pObj)->Get_State() == CStargazer::STARGAZER_STATE::FUNCTIONAL)
		{
			m_StargazerTag.push_back(static_cast<CStargazer*>(pObj)->Get_Tag());

			switch (static_cast<CStargazer*>(pObj)->Get_Tag())
			{
			case STARGAZER_TAG::OUTER:
				eButtonDesc.strCaption = TEXT("크라트 기차역 광장");
				break;

			case STARGAZER_TAG::FESTIVAL_LEADER:
				eButtonDesc.strCaption = TEXT("세라사니 길");
				break;

			case STARGAZER_TAG::FIRE_EATER:
				eButtonDesc.strCaption = TEXT("베니니 공장 중심부");
				break;
				
			default:
				break;
			}
			
			vRange = m_pGameInstance->Calc_Draw_Range(TEXT("Font_Medium"), eButtonDesc.strCaption.c_str());
			//eButtonDesc.fPadding.x = vRange.x * 1.5f ;
			eButtonDesc.fX = g_iWinSizeX * 0.125f - (fSize.x - vRange.x) * 0.5f * 0.7f;
			eButtonDesc.fSizeX = vRange.x * eButtonDesc.fFontSize ;
			eButtonDesc.fSizeY = vRange.y * eButtonDesc.fFontSize ;

			if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Button"),
				static_cast<_uint>(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_SelectLocation_Button"), &eButtonDesc)))
				return E_FAIL;

			m_pButtons.push_back(static_cast<CUI_Button*>(m_pGameInstance->Get_LastObject(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_SelectLocation_Button"))));

			eButtonDesc.fY += g_iWinSizeY * 0.05f;
		}
	}

	m_pButtons[0]->Set_isHighlight(true);

	CDynamic_UI::DYNAMIC_UI_DESC eSelectDesc = {};
	eSelectDesc.fOffset = 0.01f;
	eSelectDesc.fDuration = 0.033f;
	eSelectDesc.fSizeX = g_iWinSizeX * 0.02f;
	eSelectDesc.fSizeY = g_iWinSizeY * 0.04f;
	eSelectDesc.fX = g_iWinSizeX * 0.1f;
	eSelectDesc.fY = 0;//  g_iWinSizeY * -0.15f;
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
		static_cast<_uint>(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_SelectLocation_Button"), &eSelectDesc)))
		return E_FAIL;


	m_pSelectUI = dynamic_cast<CDynamic_UI*>(m_pGameInstance->Get_LastObject(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_SelectLocation_Button")));



	return S_OK;
}

void CUI_SelectLocation::Update_Button()
{

	if (m_pGameInstance->Key_Down(DIK_S))
	{
		if (m_iButtonIndex < m_pButtons.size() - 1)
		{
			m_pButtons[m_iButtonIndex]->Set_isHighlight(false);
			++m_iButtonIndex;
			m_pButtons[m_iButtonIndex]->Set_isHighlight(true);
		}
		CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Hovered_Default_02");

	}
	else if (m_pGameInstance->Key_Down(DIK_W))
	{
		if (m_iButtonIndex > 0)
		{
			m_pButtons[m_iButtonIndex]->Set_isHighlight(false);
			--m_iButtonIndex;
			m_pButtons[m_iButtonIndex]->Set_isHighlight(true);

		}
		CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Hovered_Default_02");
	}

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

			//CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Hovered_Default_02");
			break;
		}

	}

	if (m_pGameInstance->Key_Down(DIK_SPACE))
	{
		Interation_Button(m_iButtonIndex);
		return;
	}



	m_pButtons[m_iButtonIndex]->Set_isMouseHover(true);
	if (nullptr != m_pSelectUI)
		m_pSelectUI->Set_Position(g_iWinSizeX * 0.059f, g_iWinSizeY * (0.3f + 0.05f * m_iButtonIndex));

	static_cast<CDynamic_UI*>(m_pLocationImg->Get_PartUI()[0])->Set_iTextureIndex(ENUM_CLASS(m_StargazerTag[m_iButtonIndex]));

}

void CUI_SelectLocation::Interation_Button(_int iButtonIndex)
{
	

	CStargazer* pStar = static_cast<CStargazer*>(m_pTarget);

	pStar->Teleport_Stargazer(m_StargazerTag[iButtonIndex]);

	Set_bDead();
	
	for (auto& pButton : m_pButtons)
		pButton->Set_bDead();

	m_pSelectUI->Set_bDead();

	CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Selected_Default_03");
}

CUI_SelectLocation* CUI_SelectLocation::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_SelectLocation* pInstance = new CUI_SelectLocation(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_SelectLocation");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_SelectLocation::Clone(void* pArg)
{
	CUI_SelectLocation* pInstance = new CUI_SelectLocation(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_SelectLocation");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_SelectLocation::Free()
{
	__super::Free();

	Safe_Release(m_pLocationImg);
	Safe_Release(m_pBehindButtons);

}
