#include "UI_Levelup.h"
#include "GameInstance.h"
#include "Dynamic_Text_UI.h"
#include "Stargazer.h"
#include "Mana_Bar.h"

CUI_Levelup::CUI_Levelup(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUI_Container{pDevice, pContext}
{
}

CUI_Levelup::CUI_Levelup(const CUI_Levelup& Prototype)
	:CUI_Container{Prototype}
{
}

HRESULT CUI_Levelup::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Levelup::Initialize(void* pArg)
{
	// 플레이어 찾고
	m_pPlayer = GET_PLAYER(m_pGameInstance->GetCurrentLevelIndex());

	if (nullptr == m_pPlayer)
		return E_FAIL;

	m_eStat = m_pPlayer->Get_Stat();

	m_iLevel = m_pPlayer->Get_Player_Level();

	m_fErgo = m_pPlayer->Get_Ergo();

	m_fCurrentErgo = m_fErgo;

	// 타겟 넣어주고 
	UI_LEVELUP_DESC* pDesc = static_cast<UI_LEVELUP_DESC*>(pArg);

	m_pTarget = pDesc->pTarget;

	UI_CONTAINER_DESC eDesc = {};

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Level/Level_Background.json");

	if (FAILED(__super::Initialize(&eDesc)))
		return E_FAIL;

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Level/Ergo.json"); 

	m_pErgoInfo = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Level/Button_Levelup.json");

	m_pLevelUpButton = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Level/Ability_Info.json");

	m_pAbilInfo = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Level/Weapon_Info.json");

	m_pWeaponInfo = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Level/Legion_Info.json");

	m_pLegionInfo = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Level/Armor_Info.json");

	m_pArmorInfo = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Level/Button_RD.json");

	m_pBehindButtons = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));


	Ready_Stat_UI();

	Ready_Weapon_UI();

	static_cast<CMana_Bar*>(m_pAbilInfo->Get_PartUI().back())->Set_isUseWeapon();
	
	m_pPlayer->Callback_Mana();

	// 에르고 

	static_cast<CDynamic_Text_UI*>(m_pErgoInfo->Get_PartUI()[0])->Set_Caption(to_wstring(static_cast<_int>(Compute_MaxErgo())));
	static_cast<CDynamic_Text_UI*>(m_pErgoInfo->Get_PartUI()[1])->Set_Caption(to_wstring(static_cast<_int>(m_fCurrentErgo)));

	return S_OK;
}

void CUI_Levelup::Priority_Update(_float fTimeDelta)
{
	if (!m_bInit)
	{
		m_bInit = true;
		return;
	}


	if (m_pGameInstance->Key_Down(DIK_ESCAPE))
	{
		Set_bDead();

		CStargazer* pStar = static_cast<CStargazer*>(m_pTarget);

		pStar->Script_Activate();


		return;
	}

	if (m_pGameInstance->Key_Down(DIK_SPACE))
	{
		Set_bDead();

		CStargazer* pStar = static_cast<CStargazer*>(m_pTarget);

		pStar->End_Script();


		m_pPlayer->Set_Stat(m_eStat);
  		m_pPlayer->Apply_Stat();

		m_pPlayer->Set_Ergo(m_fCurrentErgo);

		return;
	}


	__super::Priority_Update(fTimeDelta);

	m_pErgoInfo->Priority_Update(fTimeDelta);
	m_pLevelUpButton->Priority_Update(fTimeDelta);
	m_pAbilInfo->Priority_Update(fTimeDelta);
	m_pWeaponInfo->Priority_Update(fTimeDelta);
	m_pLegionInfo->Priority_Update(fTimeDelta);
	m_pArmorInfo->Priority_Update(fTimeDelta);
	m_pBehindButtons->Priority_Update(fTimeDelta);

	for (auto& pContainer : m_pStatButtons)
		pContainer->Priority_Update(fTimeDelta);

	for (auto& pConatiner : m_pWeaponStat)
		pConatiner->Priority_Update(fTimeDelta);

	Update_Button();

}

void CUI_Levelup::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	m_pErgoInfo->Update(fTimeDelta);
	m_pLevelUpButton->Update(fTimeDelta);
	m_pAbilInfo->Update(fTimeDelta);
	m_pWeaponInfo->Update(fTimeDelta);
	m_pLegionInfo->Update(fTimeDelta);
	m_pArmorInfo->Update(fTimeDelta);
	m_pBehindButtons->Update(fTimeDelta);

	for (auto& pContainer : m_pStatButtons)
		pContainer->Update(fTimeDelta);

	for (auto& pConatiner : m_pWeaponStat)
		pConatiner->Update(fTimeDelta);
}

void CUI_Levelup::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	m_pErgoInfo->Late_Update(fTimeDelta);
	m_pLevelUpButton->Late_Update(fTimeDelta);
	m_pAbilInfo->Late_Update(fTimeDelta);
	m_pWeaponInfo->Late_Update(fTimeDelta);
	m_pLegionInfo->Late_Update(fTimeDelta);
	m_pArmorInfo->Late_Update(fTimeDelta);
	m_pBehindButtons->Late_Update(fTimeDelta);

	for (auto& pContainer : m_pStatButtons)
		pContainer->Late_Update(fTimeDelta);

	for (auto& pConatiner : m_pWeaponStat)
		pConatiner->Late_Update(fTimeDelta);
}

HRESULT CUI_Levelup::Render()
{
	return S_OK;
}


HRESULT CUI_Levelup::Ready_Stat_UI()
{
	UI_CONTAINER_DESC eDesc = {};

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Level/Select_Stat.json");

	//m_pStatButtons.push_back(static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc)));

	_float fPadding = g_iWinSizeY * 0.075f;
	for (int i = 0; i < 6; ++i)
	{
		m_pStatButtons.push_back(static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc)));

		CUI_Container* pContainer = m_pStatButtons.back();
		fPadding = g_iWinSizeY * 0.075f * (i - 1) + g_iWinSizeY * 0.075f * 0.5f;

		for (auto& pObj : pContainer->Get_PartUI())
		{
			_float2 vPos = pObj->Get_Pos();
			pObj->Set_Position(vPos.x, vPos.y + fPadding);
		}

		

		switch (i)
		{
		case 1:
			static_cast<CDynamic_Text_UI*>(pContainer->Get_PartUI()[0])->Set_Caption(L"지구력");
			break;
		case 2:
			static_cast<CDynamic_Text_UI*>(pContainer->Get_PartUI()[0])->Set_Caption(L"적재력");
			break;
		case 3:
			static_cast<CDynamic_Text_UI*>(pContainer->Get_PartUI()[0])->Set_Caption(L"동력");
			break;
		case 4:
			static_cast<CDynamic_Text_UI*>(pContainer->Get_PartUI()[0])->Set_Caption(L"기술");
			break;
		case 5:
			static_cast<CDynamic_Text_UI*>(pContainer->Get_PartUI()[0])->Set_Caption(L"진화");
			break;
		default:
			break;
		}

		if (i > 0)
		{
			// 선택 안한걸로 바꾸기
			pContainer->Get_PartUI()[0]->Set_Color({ 1.f,1.f,1.f,1.f });
			static_cast<CDynamic_UI*>(pContainer->Get_PartUI()[1])->Set_iTextureIndex(0);

			// 스탯 아이콘 맞춰서
			static_cast<CDynamic_UI*>(pContainer->Get_PartUI()[2])->Set_iTextureIndex(i);
		}
		
		static_cast<CDynamic_Text_UI*>(pContainer->Get_PartUI().back())->Set_Caption(to_wstring(m_eStat.stat[i]));
	}

	for (int i = 0; i < 6; ++i)
	{
		m_iStat[i] = m_eStat.stat[i];
	}

	

	// 불 생성
	//CDynamic_UI::DYNAMIC_UI_DESC eSelectDesc = {};
	//eSelectDesc.fOffset = 0.0f;
	//eSelectDesc.fDuration = 0.033f;
	//eSelectDesc.fSizeX = g_iWinSizeX * 0.02f;
	//eSelectDesc.fSizeY = g_iWinSizeY * 0.04f;
	//eSelectDesc.fX = g_iWinSizeX * 0.1f;
	//eSelectDesc.fY = 0;//  g_iWinSizeY * -0.15f;
	//eSelectDesc.iPassIndex = D_UI_SPRITE;
	//eSelectDesc.iTextureIndex = 0;
	//eSelectDesc.vColor = { 1.f,1.f,1.f,1.f };
	//eSelectDesc.strTextureTag = TEXT("Prototype_Component_Texture_Button_Select");
	//eSelectDesc.fAlpha = 0.5f;


	//UI_FEATURE_UV_DESC uvDesc = {};
	//uvDesc.iStartFrame = 0;
	//uvDesc.iEndFrame = 16;
	//uvDesc.fStartUV = { 0.f,0.f };
	//uvDesc.fOffsetUV = { 0.25f,0.25f };
	//uvDesc.isLoop = true;
	//uvDesc.strProtoTag = ("Prototype_Component_UI_Feature_UV");

	//eSelectDesc.FeatureDescs.push_back(&uvDesc);

	//if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Dynamic_UI"),
	//	static_cast<_uint>(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_SelectLocation_Button"), &eSelectDesc)))
	//	return E_FAIL;


	//m_pSelectUI = dynamic_cast<CDynamic_UI*>(m_pGameInstance->Get_LastObject(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_SelectLocation_Button")));

	//_float2 tempPos = m_pStatButtons[0]->Get_PartUI()[0]->Get_Pos();

	//m_pSelectUI->Set_Position(tempPos.x - g_iWinSizeX * 0.15f, tempPos.y);


	return S_OK;
}

HRESULT CUI_Levelup::Ready_Weapon_UI()
{
	// 나중에 인벤 읽어오는 거로 바꾸기
	// 일단은 그냥 생성

	UI_CONTAINER_DESC eDesc = {};

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Level/Weapon_Stat.json");

	m_pWeaponStat.push_back(static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc)));

	_float fPadding = g_iWinSizeY * 0.082f;

	m_pWeaponStat.push_back(static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc)));

	CUI_Container* pContainer = m_pWeaponStat.back();

	for (auto& pObj : pContainer->Get_PartUI())
	{
		_float2 vPos = pObj->Get_Pos();
		pObj->Set_Position(vPos.x, vPos.y + fPadding);
	}

	static_cast<CDynamic_Text_UI*>(pContainer->Get_PartUI()[1])->Set_Caption(L"리전 암");

	return S_OK;
}

void CUI_Levelup::Update_Button()
{
	if (m_pGameInstance->Key_Down(DIK_W))
	{
		if (m_iButtonIndex < 0)
		{
			m_iButtonIndex = 0;
			return;
		}
			

		auto& part0 = m_pStatButtons[m_iButtonIndex]->Get_PartUI();
		part0[0]->Set_Color({ 1,1,1,1 });
		static_cast<CDynamic_UI*>(part0[1])->Set_iTextureIndex(0);

		--m_iButtonIndex;
		if (m_iButtonIndex < 0)
		{
			m_iButtonIndex = 0;
		
		}
			

		auto& part1 = m_pStatButtons[m_iButtonIndex]->Get_PartUI();
		part1[0]->Set_Color({ 0.8f,0.1f,0.1f,1 });
		static_cast<CDynamic_UI*>(part1[1])->Set_iTextureIndex(1);
	}
	else if (m_pGameInstance->Key_Down(DIK_S))
	{
		if (m_iButtonIndex >= m_pStatButtons.size())
		{
			m_iButtonIndex = static_cast<_int>(m_pStatButtons.size()) - 1;
			return;
		}

		auto& part0 = m_pStatButtons[m_iButtonIndex]->Get_PartUI();
		part0[0]->Set_Color({ 1,1,1,1 });
		static_cast<CDynamic_UI*>(part0[1])->Set_iTextureIndex(0);

		++m_iButtonIndex;
		if (m_iButtonIndex >= m_pStatButtons.size())
		{
			m_iButtonIndex = static_cast<_int>(m_pStatButtons.size()) - 1;
			
		}
			

		auto& part1 = m_pStatButtons[m_iButtonIndex]->Get_PartUI();
		part1[0]->Set_Color({ 0.8f,0.1f,0.1f,1 });
		static_cast<CDynamic_UI*>(part1[1])->Set_iTextureIndex(1);
	}
	else if (m_pGameInstance->Key_Down(DIK_A))
	{
		Update_Stat(false);
	}
	else if (m_pGameInstance->Key_Down(DIK_D))
	{
		Update_Stat(true);
	}

}

void CUI_Levelup::Interation_Button()
{
}

void CUI_Levelup::Update_Stat(_bool isPlus)
{
	


	if (isPlus)
	{
		if (!Check_Ergo())
		{
			return;
		}

		if (m_fCurrentErgo < Compute_MaxErgo())
			return;

		m_fCurrentErgo -= Compute_MaxErgo();

		++m_iLevel;
		
		++m_iCount;


		static_cast<CDynamic_Text_UI*>(m_pErgoInfo->Get_PartUI()[0])->Set_Caption(to_wstring(static_cast<_int>(Compute_MaxErgo())));
		static_cast<CDynamic_Text_UI*>(m_pErgoInfo->Get_PartUI()[1])->Set_Caption(to_wstring(static_cast<_int>(m_fCurrentErgo)));


		auto& part = m_pStatButtons[m_iButtonIndex]->Get_PartUI().back();

		static_cast<CDynamic_Text_UI*>(part)->Set_Caption(to_wstring(++m_eStat.stat[m_iButtonIndex]));

	}
	else
	{
		if (m_iCount <= 0)
			return;

		if (m_iStat[m_iButtonIndex]  > m_eStat.stat[m_iButtonIndex] - 1)
		{
			return;
		}
		

		--m_iCount;

		--m_iLevel;
		m_fCurrentErgo += Compute_MaxErgo();

		static_cast<CDynamic_Text_UI*>(m_pErgoInfo->Get_PartUI()[0])->Set_Caption(to_wstring(static_cast<_int>(Compute_MaxErgo())));
		static_cast<CDynamic_Text_UI*>(m_pErgoInfo->Get_PartUI()[1])->Set_Caption(to_wstring(static_cast<_int>(m_fCurrentErgo)));

		auto& part = m_pStatButtons[m_iButtonIndex]->Get_PartUI().back();
		static_cast<CDynamic_Text_UI*>(part)->Set_Caption(to_wstring(--m_eStat.stat[m_iButtonIndex]));
	}
}

_bool CUI_Levelup::Check_Ergo()
{

	m_pPlayer->Check_LevelUp(m_iLevel);


	return m_pPlayer->Check_LevelUp(m_iLevel);
}

_float CUI_Levelup::Compute_MaxErgo()
{
	if (m_iLevel == 0)
	{
	
		return 100.f;
	}


	return floorf(powf(1.2f, _float(m_iLevel)) * 100.f);
}

CUI_Levelup* CUI_Levelup::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Levelup* pInstance = new CUI_Levelup(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Levelup");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Levelup::Clone(void* pArg)
{
	CUI_Levelup* pInstance = new CUI_Levelup(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_Levelup");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Levelup::Free()
{
	__super::Free();

	Safe_Release(m_pErgoInfo);
	Safe_Release(m_pLevelUpButton);
	Safe_Release(m_pAbilInfo);
	Safe_Release(m_pWeaponInfo);
	Safe_Release(m_pLegionInfo);
	Safe_Release(m_pArmorInfo);
	Safe_Release(m_pBehindButtons);

	for (auto& pContainer : m_pStatButtons)
		Safe_Release(pContainer);


	for (auto& pConatiner : m_pWeaponStat)
		Safe_Release(pConatiner);
}
