#include "UI_Levelup.h"
#include "GameInstance.h"
#include "Dynamic_Text_UI.h"
#include "Stargazer.h"
#include "Mana_Bar.h"
#include "Client_Calculation.h"
#include "Weapon.h"
#include "UI_Manager.h"

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

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Level/Confirm.json");

	m_pConfirmUI = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));


	Ready_Stat_UI();

	Ready_Weapon_UI();

	static_cast<CMana_Bar*>(m_pAbilInfo->Get_PartUI().back())->Set_isUseWeapon();
	
	m_pPlayer->Callback_Mana();

	// 에르고 

	static_cast<CDynamic_Text_UI*>(m_pErgoInfo->Get_PartUI()[0])->Set_Caption(to_wstring(static_cast<_int>(Compute_MaxErgo())));
	static_cast<CDynamic_Text_UI*>(m_pErgoInfo->Get_PartUI()[1])->Set_Caption(to_wstring(static_cast<_int>(m_fCurrentErgo)));

	// 스탯 적용해놓기

	Ready_Value();

	

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
		if (m_isRenderCorfirmUI)
		{
			m_isRenderCorfirmUI = false;
			CUI_Manager::Get_Instance()->Sound_Play("SE_UI_CloseWindow_01");
			return;
		}
		else
		{
			Set_bDead();

			CStargazer* pStar = static_cast<CStargazer*>(m_pTarget);

			pStar->Script_Activate();
			CUI_Manager::Get_Instance()->Sound_Play("SE_UI_CloseWindow_01");
			return;
		}


		
	}
	else if (m_pGameInstance->Key_Down(DIK_SPACE))
	{
		// 확인 ui 띄우기
		if (m_isRenderCorfirmUI)
		{
			m_pPlayer->Set_Player_Level(m_iLevel);
			m_pPlayer->Set_Stat(m_eStat);
			m_pPlayer->Apply_Stat();

			m_pPlayer->Set_Ergo(m_fCurrentErgo);

			m_pErgoInfo->Get_PartUI()[1]->Set_Color({ 1.f,1.f,1.f,1.f });
			
			for (auto& pContainer : m_pStatButtons)
			{
				pContainer->Get_PartUI().back()->Set_Color({ 1.f,1.f,1.f,1.f });
			}

			for (int i = 0; i < 4; ++i)
			{
				switch (i)
				{
					// 체력
				case 0:
					m_PartObjects[2]->Set_Color({ 1.f, 1.f, 1.f, 1.f });
					(m_pAbilInfo->Get_PartUI()[0])->Set_Color({ 1.f, 1.f, 1.f, 1.f });

					break;
					// 스테미나
				case 1:
					m_PartObjects[3]->Set_Color({ 1.f, 1.f, 1.f, 1.f });
					(m_pAbilInfo->Get_PartUI()[1])->Set_Color({ 1.f, 1.f, 1.f, 1.f });
					break;

				case 2:
					break;
					// 공격력
				case 3:

					(m_pWeaponInfo->Get_PartUI()[1])->Set_Color({ 1.f, 1.f, 1.f, 1.f });
					(m_pLegionInfo->Get_PartUI()[1])->Set_Color({ 1.f, 1.f, 1.f, 1.f });
					(m_pWeaponInfo->Get_PartUI()[2])->Set_Color({ 1.f, 1.f, 1.f, 1.f });
					(m_pLegionInfo->Get_PartUI()[2])->Set_Color({ 1.f, 1.f, 1.f, 1.f });

					break;
					// 공격력
				default:
					break;
				}
			}

			m_eStat = m_pPlayer->Get_Stat();

			m_iLevel = m_pPlayer->Get_Player_Level();

			m_fErgo = m_pPlayer->Get_Ergo();

			m_fCurrentErgo = m_fErgo;

			m_iCount = 0;
		

			m_pLevelUpButton->Get_PartUI()[0]->Set_Alpha(0.3f);
			m_pLevelUpButton->Get_PartUI()[1]->Set_Alpha(0.3f);
			m_pLevelUpButton->Get_PartUI()[2]->Set_Alpha(0.f);

			m_isRenderCorfirmUI = false;
		}
		else
		{
			if (m_iCount > 0)
			{
				m_isRenderCorfirmUI = true;

				_wstring strErgo = to_wstring(static_cast<_int>(m_fErgo - m_fCurrentErgo));
				static_cast<CDynamic_Text_UI*>(m_pConfirmUI->Get_PartUI().back())->Set_Caption(strErgo);

				
			}
				
		}

		CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Selected_Default_03");

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

	if (m_isRenderCorfirmUI)
		m_pConfirmUI->Priority_Update(fTimeDelta);

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

	if (m_isRenderCorfirmUI)
		m_pConfirmUI->Update(fTimeDelta);
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

	if (m_isRenderCorfirmUI)
		m_pConfirmUI->Late_Update(fTimeDelta);
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

HRESULT CUI_Levelup::Ready_Value()
{
	_float fValue = {};
	_wstring strValue = {};
	// 스탯을 기반으로 값을 다 바꾸기
	// 다 만들고 처음에도 넣어주기
	_float fBaseDamage = m_pPlayer->Get_Equip_Weapon()->GetBaseDamage();

	_float fLegionBaseDamage = m_pPlayer->Get_Equip_Legion()->GetBaseDamage();

	// 체력
	fValue = floorf(ComputeLog(_float(m_eStat.iVitality), 2) * 100.f);
	strValue = to_wstring(_int(fValue)) + L" / " + to_wstring(_int(fValue));
	static_cast<CDynamic_Text_UI*>(m_pAbilInfo->Get_PartUI()[0])->Set_Caption(strValue);

	// 스태
	fValue = floorf(ComputeLog(_float(m_eStat.iStamina), 5) * 100.f);
	strValue = to_wstring(_int(fValue)) + L" / " + to_wstring(_int(fValue));
	static_cast<CDynamic_Text_UI*>(m_pAbilInfo->Get_PartUI()[1])->Set_Caption(strValue);

	// 무기, 리전 공격력
	fValue = floorf(fBaseDamage * (ComputeLog(_float(m_eStat.iMotivity), 10)) * 0.5f) + floorf(fBaseDamage * (ComputeLog(_float(m_eStat.iTechnique), 10)) * 0.5f);
	strValue = to_wstring(_int(fBaseDamage)) + L" + " + to_wstring(_int(fValue));
	static_cast<CDynamic_Text_UI*>(m_pWeaponInfo->Get_PartUI()[2])->Set_Caption(strValue);

	fValue = floorf(fLegionBaseDamage * (ComputeLog(_float(m_eStat.iMotivity), 10)) * 0.1f) + floorf(fLegionBaseDamage * (ComputeLog(_float(m_eStat.iTechnique), 10)) * 0.1f);
	strValue = to_wstring(_int(fLegionBaseDamage)) + L" + " + to_wstring(_int(fValue));
	static_cast<CDynamic_Text_UI*>(m_pLegionInfo->Get_PartUI()[2])->Set_Caption(strValue);

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


		if (m_iButtonIndex != m_pStatButtons.size())
		{
			auto& part0 = m_pStatButtons[m_iButtonIndex]->Get_PartUI();
			part0[0]->Set_Color({ 1,1,1,1 });
			static_cast<CDynamic_UI*>(part0[1])->Set_iTextureIndex(0);
		}
		else
		{
			m_pLevelUpButton->Get_PartUI()[2]->Set_Alpha(0.f);
		}

		--m_iButtonIndex;
		if (m_iButtonIndex < 0)
		{
			m_iButtonIndex = 0;
		
		}
			

		auto& part1 = m_pStatButtons[m_iButtonIndex]->Get_PartUI();
		part1[0]->Set_Color({ 0.8f,0.1f,0.1f,1 });
		static_cast<CDynamic_UI*>(part1[1])->Set_iTextureIndex(1);


		CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Hovered_Default_02");

	}
	else if (m_pGameInstance->Key_Down(DIK_S))
	{
		// 레벨업 버튼으로 가도록 처리하자
		if (m_iButtonIndex > m_pStatButtons.size())
		{
			m_iButtonIndex = static_cast<_int>(m_pStatButtons.size());
			return;
		}

		// 일단 레벨업이 안되는 경우는 처리 안하기
		if (m_iButtonIndex >= static_cast<_int>(m_pStatButtons.size()))
		{
			if (m_iCount == 0)
				return;
			auto& part0 = m_pStatButtons.back()->Get_PartUI();
			part0[0]->Set_Color({ 1,1,1,1 });
			static_cast<CDynamic_UI*>(part0[1])->Set_iTextureIndex(0);

			m_pLevelUpButton->Get_PartUI()[2]->Set_Alpha(1.f);
			
		}
		else
		{
			auto& part0 = m_pStatButtons[m_iButtonIndex]->Get_PartUI();
			part0[0]->Set_Color({ 1,1,1,1 });
			static_cast<CDynamic_UI*>(part0[1])->Set_iTextureIndex(0);
			++m_iButtonIndex;
			
			if (m_iButtonIndex == static_cast<_int>(m_pStatButtons.size()))
			{
				if (m_iCount == 0)
				{
					--m_iButtonIndex;
					auto& part1 = m_pStatButtons[m_iButtonIndex]->Get_PartUI();
					part1[0]->Set_Color({ 0.8f,0.1f,0.1f,1 });
					static_cast<CDynamic_UI*>(part1[1])->Set_iTextureIndex(1);
					return;
				}
					
					auto& part0 = m_pStatButtons.back()->Get_PartUI();
					part0[0]->Set_Color({ 1,1,1,1 });
					static_cast<CDynamic_UI*>(part0[1])->Set_iTextureIndex(0);

					m_pLevelUpButton->Get_PartUI()[2]->Set_Alpha(1.f);
			}
			else
			{
				auto& part1 = m_pStatButtons[m_iButtonIndex]->Get_PartUI();
				part1[0]->Set_Color({ 0.8f,0.1f,0.1f,1 });
				static_cast<CDynamic_UI*>(part1[1])->Set_iTextureIndex(1);
			}

			CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Hovered_Default_02");
		}

		
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
	if (m_iButtonIndex == m_pStatButtons.size())
		return;

	_int SelectIndex = {};


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

		m_pLevelUpButton->Get_PartUI()[0]->Set_Alpha(1.f);
		m_pLevelUpButton->Get_PartUI()[1]->Set_Alpha(1.f);


		static_cast<CDynamic_Text_UI*>(m_pErgoInfo->Get_PartUI()[0])->Set_Caption(to_wstring(static_cast<_int>(Compute_MaxErgo())));
		static_cast<CDynamic_Text_UI*>(m_pErgoInfo->Get_PartUI()[1])->Set_Caption(to_wstring(static_cast<_int>(m_fCurrentErgo)));
		m_pErgoInfo->Get_PartUI()[1]->Set_Color({ 1.f,0.3f,0.1f,1.f });


		auto& part = m_pStatButtons[m_iButtonIndex]->Get_PartUI().back();

		static_cast<CDynamic_Text_UI*>(part)->Set_Caption(to_wstring(++m_eStat.stat[m_iButtonIndex]));
		SelectIndex = m_iButtonIndex;

		m_pStatButtons[SelectIndex]->Get_PartUI().back()->Set_Color({ 0.08f,0.5f,0.5f,1.f });
		switch (SelectIndex)
		{
			// 체력
		case 0:
			m_PartObjects[2]->Set_Color({ 0.08f,0.5f,0.5f,1.f });
			(m_pAbilInfo->Get_PartUI()[0])->Set_Color({ 0.08f,0.5f,0.5f,1.f });

			break;
			// 스테미나
		case 1:
			m_PartObjects[3]->Set_Color({ 0.08f,0.5f,0.5f,1.f });
			(m_pAbilInfo->Get_PartUI()[1])->Set_Color({ 0.08f,0.5f,0.5f,1.f });
			break;

		case 2:
			break;
			// 공격력
		case 3:

			(m_pWeaponInfo->Get_PartUI()[1])->Set_Color({ 0.08f,0.5f,0.5f,1.f });
			(m_pLegionInfo->Get_PartUI()[1])->Set_Color({ 0.08f,0.5f,0.5f,1.f });
			(m_pWeaponInfo->Get_PartUI()[2])->Set_Color({ 0.08f,0.5f,0.5f,1.f });
			(m_pLegionInfo->Get_PartUI()[2])->Set_Color({ 0.08f,0.5f,0.5f,1.f });

			break;
			// 공격력
		case 4:

			(m_pWeaponInfo->Get_PartUI()[1])->Set_Color({ 0.08f,0.5f,0.5f,1.f });
			(m_pLegionInfo->Get_PartUI()[1])->Set_Color({ 0.08f,0.5f,0.5f,1.f });
			(m_pWeaponInfo->Get_PartUI()[2])->Set_Color({ 0.08f,0.5f,0.5f,1.f });
			(m_pLegionInfo->Get_PartUI()[2])->Set_Color({ 0.08f,0.5f,0.5f,1.f });
			break;
			// 팔 공격력만? 일단 보류
		case 5:
			break;
		default:
			break;
		}
	
	}
	else
	{
		if (m_iCount <= 0)
		{
			m_pErgoInfo->Get_PartUI()[1]->Set_Color({ 1.f,1.f,1.f,1.f });

			return;
		}
			

		if (m_iStat[m_iButtonIndex]  > m_eStat.stat[m_iButtonIndex] - 1)
		{

			m_pStatButtons[m_iButtonIndex]->Get_PartUI().back()->Set_Color({ 1.f,1.f,1.f,1.f });

			return;
		}
		

		--m_iCount;

		--m_iLevel;
		m_fCurrentErgo += Compute_MaxErgo();

		static_cast<CDynamic_Text_UI*>(m_pErgoInfo->Get_PartUI()[0])->Set_Caption(to_wstring(static_cast<_int>(Compute_MaxErgo())));
		static_cast<CDynamic_Text_UI*>(m_pErgoInfo->Get_PartUI()[1])->Set_Caption(to_wstring(static_cast<_int>(m_fCurrentErgo)));

		auto& part = m_pStatButtons[m_iButtonIndex]->Get_PartUI().back();
		static_cast<CDynamic_Text_UI*>(part)->Set_Caption(to_wstring(--m_eStat.stat[m_iButtonIndex]));

		SelectIndex = m_iButtonIndex;

		if (m_iStat[SelectIndex] == m_eStat.stat[SelectIndex])
		{
			switch (SelectIndex)
			{
				// 체력
			case 0:
				m_PartObjects[2]->Set_Color({ 0.7f, 0.7f, 0.7f, 1.f });
				(m_pAbilInfo->Get_PartUI()[0])->Set_Color({ 0.7f, 0.7f, 0.7f, 1.f });

				break;
				// 스테미나
			case 1:
				m_PartObjects[3]->Set_Color({ 0.7f, 0.7f, 0.7f, 1.f });
				(m_pAbilInfo->Get_PartUI()[1])->Set_Color({ 0.7f, 0.7f, 0.7f, 1.f });
				break;

			case 2:
				break;
				// 공격력
			case 3:

				(m_pWeaponInfo->Get_PartUI()[1])->Set_Color({ 0.7f, 0.7f, 0.7f, 1.f });
				(m_pLegionInfo->Get_PartUI()[1])->Set_Color({ 0.7f, 0.7f, 0.7f, 1.f });
				(m_pWeaponInfo->Get_PartUI()[2])->Set_Color({ 0.7f, 0.7f, 0.7f, 1.f });
				(m_pLegionInfo->Get_PartUI()[2])->Set_Color({ 0.7f, 0.7f, 0.7f, 1.f });

				break;
				// 공격력
			case 4:
				(m_pWeaponInfo->Get_PartUI()[1])->Set_Color({ 0.7f, 0.7f, 0.7f, 1.f });
				(m_pLegionInfo->Get_PartUI()[1])->Set_Color({ 0.7f, 0.7f, 0.7f, 1.f });
				(m_pWeaponInfo->Get_PartUI()[2])->Set_Color({ 0.7f, 0.7f, 0.7f, 1.f });
				(m_pLegionInfo->Get_PartUI()[2])->Set_Color({ 0.7f, 0.7f, 0.7f, 1.f });
				break;
				// 팔 공격력만? 일단 보류
			case 5:
				break;
			default:
				break;
			}
		}

		if (m_iCount == 0)
		{
			m_pLevelUpButton->Get_PartUI()[0]->Set_Alpha(0.3f);
			m_pLevelUpButton->Get_PartUI()[1]->Set_Alpha(0.3f);
			m_pLevelUpButton->Get_PartUI()[2]->Set_Alpha(0.f);

			m_pStatButtons[SelectIndex]->Get_PartUI().back()->Set_Color({ 1.f,1.f,1.f,1.f });
			
			m_pErgoInfo->Get_PartUI()[1]->Set_Color({ 1.f,1.f,1.f,1.f });
		}
	}

	_float fValue = {};
	_wstring strValue = {};
	// 스탯을 기반으로 값을 다 바꾸기
	// 다 만들고 처음에도 넣어주기
	_float fBaseDamage = m_pPlayer->Get_Equip_Weapon()->GetBaseDamage();

	_float fLegionBaseDamage = m_pPlayer->Get_Equip_Legion()->GetBaseDamage();
	switch (SelectIndex)
	{
		// 체력
	case 0:
		fValue = floorf(ComputeLog(_float(m_eStat.iVitality), 2) * 100.f);
		strValue = to_wstring(_int(fValue)) + L" / " + to_wstring(_int(fValue));
		static_cast<CDynamic_Text_UI*>(m_pAbilInfo->Get_PartUI()[0])->Set_Caption(strValue);
		break;
		// 스테미나
	case 1:
		fValue = floorf(ComputeLog(_float(m_eStat.iStamina), 5) * 100.f);
		strValue = to_wstring(_int(fValue)) + L" / " + to_wstring(_int(fValue));
		static_cast<CDynamic_Text_UI*>(m_pAbilInfo->Get_PartUI()[1])->Set_Caption(strValue);
		break;
		
	case 2:
		break;
		// 공격력
	case 3:
		fValue = floorf(fBaseDamage * (ComputeLog(_float(m_eStat.iMotivity), 10)) * 0.5f) + floorf(fBaseDamage * (ComputeLog(_float(m_eStat.iTechnique), 10)) * 0.5f);
		strValue = to_wstring(_int(fBaseDamage)) + L" + " + to_wstring(_int(fValue));
		static_cast<CDynamic_Text_UI*>(m_pWeaponInfo->Get_PartUI()[2])->Set_Caption(strValue);

		fValue = floorf(fLegionBaseDamage * (ComputeLog(_float(m_eStat.iMotivity), 10)) * 0.1f) + floorf(fLegionBaseDamage * (ComputeLog(_float(m_eStat.iTechnique), 10)) * 0.1f);
		strValue = to_wstring(_int(fLegionBaseDamage)) + L" + " + to_wstring(_int(fValue));
		static_cast<CDynamic_Text_UI*>(m_pLegionInfo->Get_PartUI()[2])->Set_Caption(strValue);

		break;
		// 공격력
	case 4:
		fValue = floorf(fBaseDamage * (ComputeLog(_float(m_eStat.iMotivity), 10)) * 0.5f) + floorf(fBaseDamage * (ComputeLog(_float(m_eStat.iTechnique), 10)) * 0.5f);
		strValue = to_wstring(_int(fBaseDamage)) + L" + " + to_wstring(_int(fValue));
		static_cast<CDynamic_Text_UI*>(m_pWeaponInfo->Get_PartUI()[2])->Set_Caption(strValue);

		fValue = floorf(fLegionBaseDamage * (ComputeLog(_float(m_eStat.iMotivity), 10)) * 0.1f) + floorf(fLegionBaseDamage * (ComputeLog(_float(m_eStat.iTechnique), 10)) * 0.1f);
		strValue = to_wstring(_int(fLegionBaseDamage)) + L" + " + to_wstring(_int(fValue));
		static_cast<CDynamic_Text_UI*>(m_pLegionInfo->Get_PartUI()[2])->Set_Caption(strValue);
		break;
		// 팔 공격력만? 일단 보류
	case 5:
		break;
	default:
		break;
	}
	
	
	
	CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Selected_Default_03");

}

_bool CUI_Levelup::Check_Ergo()
{


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
	Safe_Release(m_pConfirmUI);

	for (auto& pContainer : m_pStatButtons)
		Safe_Release(pContainer);


	for (auto& pConatiner : m_pWeaponStat)
		Safe_Release(pConatiner);
}
