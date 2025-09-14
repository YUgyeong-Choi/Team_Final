#include "Loader.h"

#include "GameInstance.h"

#include "Camera_Free.h"
#include "Sky.h"
#include "UI_Button.h"

#include <future>

#pragma region LEVEL_KRAT_CENTERAL_STATION
#include "StaticMesh.h"
#include "StaticMesh_Instance.h"
#include "Nav.h"
#include "Static_Decal.h"
#include "Stargazer.h"
#include "StargazerEffect.h"
#include "ErgoItem.h"
#include "BreakableMesh.h"
#pragma endregion

#pragma region LEVEL_KRAT_HOTEL

#pragma endregion

#pragma region LEVEL_YW
#include "DecalToolObject.h"
#include "MapToolObject.h"
#include "PreviewObject.h"
#include "MonsterToolObject.h"

#include "MapLoader.h"
#pragma endregion

#pragma region LEVEL_CY
#include "ToolSprite.h"
#include "ToolParticle.h"
#include "ToolMeshEffect.h"
#include "ToolTrail.h"
#include "Effect_Manager.h"
#pragma endregion


#pragma region LEVEL_YG
#include "YGObject.h"
#include "YGCapsule.h"
#include "YGTriangleMesh.h"
#include "YGConvexMesh.h"
#include "YGBox.h"
#include "YGShpere.h"
#include "YGTrrigerWithoutModel.h"
#include "YGFloor.h"
#include "YGDynamicGib.h"
#include "YGDynamicObj.h"
#include "YGController.h"

#include "Wego.h"
#include "SlideDoor.h"
#include "KeyDoor.h"
#include "BossDoor.h"
#include "ShortCutDoor.h"
#include "TriggerSound.h"
#include "TriggerTalk.h"
#include "TriggerUI.h"
#include "TriggerBGM.h"
#include "TriggerItemLamp.h"
#pragma endregion

#pragma region LEVEL_DH
#include "PBRMesh.h"
#include "DH_ToolMesh.h"
#include "Player.h"
#include "Bayonet.h"
#include "PlayerLamp.h"
#include "PlayerFrontCollider.h"
#include "WaterPuddle.h"
#pragma endregion

#pragma region LEVEL_GL
#include "UI_Container.h"
#include "Dynamic_UI.h"
#include "Dynamic_Text_UI.h"
#include "UI_Text.h"
#include "UI_Guide.h"
#include "HP_Bar.h"
#include "Mana_Bar.h"
#include "Stamina_Bar.h"
#include "Durability_Bar.h"
#include "Egro_Bar.h"
#include "Legion_Bar.h"
#include "Icon_Item.h"
#include "Panel_Player_LD.h"
#include "Panel_Player_RD.h"
#include "Panel_Player_RU.h"
#include "Panel_Player_Arm.h"
#include "Belt.h"
#include "Lamp.h"
#include "Grinder.h"
#include "Portion.h"
#include "Icon_Weapon.h"
#include "Icon_LegionArm.h"
#include "Monster_Test.h"
#include "Weapon_Monster.h"
#include "Buttler_Train.h"
#include "UI_MonsterHP_Bar.h"
#include "UI_LockOn_Icon.h"
#include "UI_Fatal_Icon.h"
#include "LegionArm_Steel.h"
#include "UI_Popup.h"
#include "UI_Script_Text.h"
#include "UI_Script_Talk.h"
#include "ActionType_Icon.h"
#include "UI_SelectWeapon.h"
#include "UI_Pickup_Item.h"
#include "Buttler_Basic.h"
#include "UI_Button_Script.h"
#include "Buttler_Range.h"
#include "Bullet.h"
#include "DeBuff_Bar.h"
#include "UI_Container_DeBuff.h"
#include "WatchDog.h"
#include "UI_Script_StarGazer.h"
#include "UI_SelectLocation.h"
#include "UI_Levelup.h"
#pragma endregion

#pragma region LEVEL_JW
#include "Oil.h"
#include "Fuoco.h"
#include "FireBall.h"
#include "FlameField.h"
#include "Elite_Police.h"
#include "AnimatedProp.h"
#include "FestivalLeader.h"
#include "TestAnimObject.h"
#include "MapLoader.h"
#pragma endregion

_bool CLoader::m_bLoadStatic = false;

CLoader::CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, atomic<float>& fRatio)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::Get_Instance() }
	, m_fRatio{fRatio}
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pDevice);
}

_uint APIENTRY LoadingMain(void* pArg)
{
	/* 자원로딩한다. */
	CLoader*		pLoader = static_cast<CLoader*>(pArg);

	if (FAILED(pLoader->Loading()))
		return 1;	

	return 0;

}

HRESULT CLoader::Initialize(LEVEL eNextLevelID)
{
	m_eNextLevelID = eNextLevelID;

	InitializeCriticalSection(&m_CriticalSection);

	m_hThread = (HANDLE)_beginthreadex(nullptr, 0, LoadingMain, this, 0, nullptr);
	if (0 == m_hThread)
		return E_FAIL;

	m_pMapLoader = CMapLoader::Create(m_pDevice, m_pContext);

	return S_OK;
}

HRESULT CLoader::Loading()
{
	CoInitializeEx(nullptr, 0);	

	EnterCriticalSection(&m_CriticalSection);

	HRESULT		hr = {};

	switch (m_eNextLevelID)
	{
	case LEVEL::LOGO:
		if(!m_bLoadStatic)
			hr = Loading_For_Static();
		hr = Loading_For_Logo();
		break;

	case LEVEL::KRAT_CENTERAL_STATION:
		hr = Loading_For_KRAT_CENTERAL_STATION();
		break;
	case LEVEL::KRAT_HOTEL:
		hr = Loading_For_KRAT_HOTEL();
		break;
	case LEVEL::DH:
		hr = Loading_For_DH();
		break;
	case LEVEL::JW:
		hr = Loading_For_JW();
		break;
	case LEVEL::GL:
		hr = Loading_For_GL();
		break;
	case LEVEL::YW:
		hr = Loading_For_YW();
		break;
	case LEVEL::CY:
		hr = Loading_For_CY();
		break;
	case LEVEL::YG:
		hr = Loading_For_YG();
		break;
	}

	if (FAILED(hr))
	{
		MSG_BOX("로딩 실패");
		return E_FAIL;
	}


	LeaveCriticalSection(&m_CriticalSection);



	return S_OK;
}

HRESULT CLoader::Loading_For_Logo()
{	
	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));

	/* For.Prototype_Component_Texture_TeamPicture*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::LOGO), TEXT("Prototype_Component_Texture_TeamPicture"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Guide/TeamPicture.dds")))))
		return E_FAIL;


	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));

	lstrcpy(m_szLoadingText, TEXT("셰이더을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));
	

	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));

	
	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_Static()
{
	m_bLoadStatic = true;
	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));

	Loading_For_UI_Texture();

	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));

	_matrix PreTransformMatrix = XMMatrixIdentity();
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_SkyBox"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Sky/SkyBox.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_DirrectionalLight"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/DirrectionalLight.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_PointLight"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/PointLight.bin", PreTransformMatrix))))
		return E_FAIL;

	_matrix		PreTransformMatrixSpot = XMMatrixIdentity();
	PreTransformMatrixSpot = XMMatrixRotationY(XMConvertToRadians(90.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_SpotLight"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SpotLight.bin", PreTransformMatrixSpot))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
		CPBRMesh::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("셰이더을(를) 로딩중입니다."));
	

	lstrcpy(m_szLoadingText, TEXT("이펙트을(를) 로딩중입니다."));
	if (FAILED(CEffect_Manager::Get_Instance()->Initialize(m_pDevice, m_pContext, TEXT("../Bin/Save/Effect/EffectContainer"))))
		return E_FAIL;
	if (FAILED(CEffect_Manager::Get_Instance()->Ready_Effect(TEXT("../Bin/Save/Effect/TE_Test_20_30_3.json"))))
		return E_FAIL;
	if (FAILED(CEffect_Manager::Get_Instance()->Ready_Effect(TEXT("../Bin/Save/Effect/TE_Skill.json"))))
		return E_FAIL;
	if (FAILED(CEffect_Manager::Get_Instance()->Ready_Effect(TEXT("../Bin/Save/Effect/TE_BloodTest.json"))))
		return E_FAIL;
	if (FAILED(CEffect_Manager::Get_Instance()->Ready_Effect(TEXT("../Bin/Save/Effect/PE_Player_SkillWeaponParticle.json"))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_ToolMesh"),
		CDH_ToolMesh::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Button"),
		CUI_Button::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Text"),
		CUI_Text::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Dynamic_Text_UI"),
		CDynamic_Text_UI::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"),
		CUI_Container::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Guide"),
		CUI_Guide::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_HP_Bar"),
		CHP_Bar::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Mana_Bar"),
		CMana_Bar::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Stamina_Bar"),
		CStamina_Bar::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Durability_Bar"),
		CDurability_Bar::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Ergo_Bar"),
		CErgo_Bar::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Legion_Bar"),
		CLegion_Bar::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_DeBuff_Bar"),
		CDeBuff_Bar::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Panel_Player_LD"),
		CPanel_Player_LD::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Panel_Player_RD"),
		CPanel_Player_RD::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Panel_Player_RU"),
		CPanel_Player_RU::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Panel_Player_Arm"),
		CPanel_Player_Arm::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Icon_Item"),
		CIcon_Item::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Icon_Weapon"),
		CIcon_Weapon::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Icon_ActionType"),
		CActionType_Icon::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Icon_LegionArm"),
		CIcon_LegionArm::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Belt"),
		CBelt::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Lamp"),
		CLamp::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Grinder"),
		CGrinder::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Portion"),
		CPortion::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_LockOn_Icon"),
		CUI_LockOn_Icon::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Fatal_Icon"),
		CUI_Fatal_Icon::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_LegionArm_Steel"),
		CLegionArm_Steel::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Popup"),
		CUI_Popup::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Script_Text"),
		CUI_Script_Text::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Script_Talk"),
		CUI_Script_Talk::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_SelectWeapon"),
		CUI_SelectWeapon::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Pickup_Item"),
		CUI_Pickup_Item::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Button_Script"),
		CUI_Button_Script::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container_DeBuff"),
		CUI_Container_DeBuff::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Script_Stargazer"),
		CUI_Script_StarGazer::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_SelectLocation"),
		CUI_SelectLocation::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Levelup"),
		CUI_Levelup::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	return S_OK;
}

HRESULT CLoader::Loading_For_KRAT_CENTERAL_STATION()
{

	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));

	//땅찍기 데칼
	/* For.Prototype_Component_Texture_FireEater_Slam_Mask*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Texture_FireEater_Slam_Mask"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Decal/T_Decal_01_C_KMH.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_FireEater_Slam_Normal*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Texture_FireEater_Slam_Normal"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Decal/T_Decal_01_N_KMH.dds")))))
		return E_FAIL;

	//스크래치 데칼
	/* For.Prototype_Component_Texture_FireEater_Scratch_Mask*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Texture_FireEater_Scratch_Mask"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Decal/T_Decal_12_C_KMH.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_FireEater_Scratch_Normal*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Texture_FireEater_Scratch_Normal"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Decal/T_Decal_12_N_KMH.dds")))))
		return E_FAIL;

	m_fRatio = 0.1f;

	lstrcpy(m_szLoadingText, TEXT("셰이더을(를) 로딩중입니다."));
	m_fRatio = 0.2f;

	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));

	/* [ 메인 플레이어 로딩 ] */
	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(-90.f));

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Player"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Player/Player.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_PlayerWeapon"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Weapon/Bayonet.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_PlayerLamp"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Lamp.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_WaterPuddleA"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/WaterPuddleA.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Elite_Police"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Elite_Police/Elite_Police.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_Elite_Police_Weapon"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Club/Elite_Police_Weapon.bin", PreTransformMatrix))))
		return E_FAIL;	
	
	

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_Wego"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Wego/Wego.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_FireEater"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FireEater/FireEater.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_FestivalLeader"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FestivalLeader/FestivalLeader.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_Buttler_Train_Weapon"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Weapon_Buttler/SK_WP_MOB_ButtlerTrain_01.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_Buttler_Basic_Weapon"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Candle/SK_WP_MOB_Candle_01.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_Buttler_Range_Weapon"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/ShotGun/SK_WP_MOB_Shotgun_01.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_FestivalWeapon"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FestivalLeader/Weapon/FestivalWeapon.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_Elite_Police_Weapon"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Club/Elite_Police_Weapon.bin", PreTransformMatrix))))
		return E_FAIL;

	//별바라기 모델
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Stargazer"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Stargazer/Stargazer.bin", PreTransformMatrix))))
		return E_FAIL;
	//별바라기 모델 부서진거
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Stargazer_Destroyed"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Stargazer_Des_2/Stargazer_Des_2.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.0115f, 0.0115f, 0.0115f) * XMMatrixRotationY(XMConvertToRadians(-90.f));

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Buttler_Train"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Buttler_Train/Buttler_Train.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Buttler_Basic"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Buttler_Basic/Buttler_Basic.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Buttler_Range"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Buttler_Range/Buttler_Range.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_WatchDog"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/WatchDog/WatchDog.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Buttler_Train"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Buttler_Train/Buttler_Train.bin", PreTransformMatrix))))
		return E_FAIL;
	
	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.004f, 0.004f, 0.004f);

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Train"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Train.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Station"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Station.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Hotel"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Hotel.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_OutDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/OutDoor.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_SquareStatue"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/SquareStatue/SquareStatue.bin", PreTransformMatrix))))
		return E_FAIL;
	PreTransformMatrix = XMMatrixScaling(0.012f, 0.012f, 0.012f) * XMMatrixRotationY(XMConvertToRadians(90.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_ClownPanel"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/ClownPanel/ClownPanel.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_ClownStationPanel"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/ClownStationPanel/ClownStationPanel.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixScaling(0.004f, 0.004f, 0.004f);
	/* 상호 작용 문 */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_SM_Station_TrainDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_Station_TrainDoor_01.bin", PreTransformMatrix))))
		return E_FAIL;
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(-180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_StationInnerDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/StationInnerDoor/StationInnerDoor.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_FacotoryDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FacotoryDoor/FacotoryDoor.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix *= XMMatrixRotationY(XMConvertToRadians(-180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_FestivalDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FestivalDoor/FestivalDoor.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_FestivalCrashDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FestivalCrashDoor/FestivalCrashDoor.bin", PreTransformMatrix))))
		return E_FAIL;


	PreTransformMatrix = XMMatrixScaling(0.011f, 0.011f, 0.011f);
	PreTransformMatrix *= XMMatrixRotationY(XMConvertToRadians(-180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_StationDoubleDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/StationDoubleDoor/StationDoubleDoor.bin", PreTransformMatrix))))
		return E_FAIL;


	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	PreTransformMatrix *= XMMatrixRotationY(XMConvertToRadians(-270.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_ShortCutDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/ShortCutDoor/ShortCutDoor.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	PreTransformMatrix *= XMMatrixRotationY(XMConvertToRadians(-90.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_HeavyLock"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/HeavyLock/HeavyLock.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	PreTransformMatrix *= XMMatrixRotationY(XMConvertToRadians(-270.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_HeavyLockSmall"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/HeavyLockSmall/HeavyLockSmall.bin", PreTransformMatrix))))
		return E_FAIL;

	m_fRatio = 0.4f;


	lstrcpy(m_szLoadingText, TEXT("네비게이션을(를) 로딩중입니다."));

	m_fRatio = 0.6f;
	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));

	m_fRatio = 0.7f;
	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));

	/* [ 메인 플레이어 로딩 ] */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Player"),
		CPlayer::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_PlayerWeapon"),
		CBayonet::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_PlayerLamp"),
		CPlayerLamp::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_PlayerFrontCollider"),
		CPlayerFrontCollider::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Wego"),
		CWego::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Elite_Police"),
		CElite_Police::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Bullet"),
		CBullet::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_WaterPuddle"),
		CWaterPuddle::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_FireBall"),
		CFireBall::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Oil"),
		COil::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_FlameField"),
		CFlameField::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	//뒤에 몬스터 붙이는거 뺐어요(영웅) TEXT("Prototype_GameObject_Buttler_Monster_Train") 이거
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Test"),
		CMonster_Test::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Buttler_Train"),
		CButtler_Train::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Buttler_Basic"),
		CButtler_Basic::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Buttler_Range"),
		CButtler_Range::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_WatchDog"),
		CWatchDog::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	//FIRE EATER로 바꿧어요 (영웅)
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_FireEater"),
		CFuoco::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_FestivalLeader"),
		CFestivalLeader::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_Weapon"),
		CWeapon_Monster::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_HPBar"),
		CUI_MonsterHP_Bar::Create(m_pDevice, m_pContext))))
		return E_FAIL;

#pragma region 트리거용

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_TriggerTalk"),
		CTriggerTalk::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_TriggerSound"),
		CTriggerSound::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_TriggerUI"),
		CTriggerUI::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_TriggerBGM"),
		CTriggerBGM::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_TriggerItemLamp"),
		CTriggerItemLamp::Create(m_pDevice, m_pContext))))
		return E_FAIL;
#pragma endregion

	/* 상호 작용 문 */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_SlideDoor"),
		CSlideDoor::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_KeyDoor"),
		CKeyDoor::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_BossDoor"),
		CBossDoor::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_ShortCutDoor"),
		CShortCutDoor::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AnimatedProp"),
		CAnimatedProp::Create(m_pDevice, m_pContext))))
		return E_FAIL;

#pragma region YW

	//부술수있는 메쉬 테스트용 모델
	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.004f, 0.004f, 0.004f);

	//파트2
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Part2"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_Factory_Pipe_01_PipePillar_6_f7.bin", PreTransformMatrix))))
		return E_FAIL;

	//파트1
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Part1"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_Factory_Pipe_01_PipePillar_6_f6.bin", PreTransformMatrix))))
		return E_FAIL;
	//SM_Interior_Table_07_3of8
	//본 모델
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Main"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_Factory_BasePipe_07.bin", PreTransformMatrix))))
		return E_FAIL;


	//부술수 있는 메쉬(푸오코 기둥 등...)
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_BreakableMesh"),
		CBreakableMesh::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	//에르고아이템(꽃)
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_ErgoItem"),
		CErgoItem::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	//별바라기
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Stargazer"),
		CStargazer::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	//스태틱 데칼	
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Static_Decal"),
		CStatic_Decal::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	//네비게이션 컴포넌트 작동시켜주는 녀석
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Nav"),
		CNav::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_StaticMesh"),
		CStaticMesh::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_StaticMesh_Instance"),
		CStaticMesh_Instance::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	// 별바라기 전용 이펙트 세트
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_StargazerEffect"),
		CStargazerEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

#pragma region 맵 로딩

	lstrcpy(m_szLoadingText, TEXT("맵 생성 시작!!..."));

	m_pGameInstance->ClaerOctoTreeObjects();


#ifdef TESTMAP

#ifdef TEST_TEST_MAP
	//네비 소환
	if (FAILED(m_pMapLoader->Loading_Navigation(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "TEST")))
		return E_FAIL;
	if (FAILED(m_pMapLoader->Ready_Nav(TEXT("Layer_Nav"), ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "TEST")))
		return E_FAIL;

	if (FAILED(m_pMapLoader->Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "TEST")))
		return E_FAIL;
	if (FAILED(m_pMapLoader->Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "TEST")))
		return E_FAIL;
#endif // TEST_TEST_MAP

#ifdef TEST_STATION_MAP

	//네비 소환
	if (FAILED(m_pMapLoader->Loading_Navigation(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "STATION")))
		return E_FAIL;
	if (FAILED(m_pMapLoader->Ready_Nav(TEXT("Layer_Nav"), ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "STATION")))
		return E_FAIL;

	if (FAILED(m_pMapLoader->Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "STATION")))
		return E_FAIL;
	if (FAILED(m_pMapLoader->Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "STATION")))
		return E_FAIL;
#endif // TEST_STATION_MAP

#ifdef TEST_HOTEL_MAP

	//네비 소환
	if (FAILED(m_pMapLoader->Loading_Navigation(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "HOTEL")))
		return E_FAIL;
	if (FAILED(m_pMapLoader->Ready_Nav(TEXT("Layer_Nav"), ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "HOTEL")))
		return E_FAIL;

	if (FAILED(m_pMapLoader->Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "HOTEL")))
		return E_FAIL;
	if (FAILED(m_pMapLoader->Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "HOTEL")))
		return E_FAIL;
#endif // TEST_HOTEL_MAP

#ifdef TEST_OUTER_MAP

	//네비 소환
	if (FAILED(m_pMapLoader->Loading_Navigation(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "OUTER")))
		return E_FAIL;
	if (FAILED(m_pMapLoader->Ready_Nav(TEXT("Layer_Nav"), ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "OUTER")))
		return E_FAIL;

	if (FAILED(m_pMapLoader->Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "OUTER")))
		return E_FAIL;
	if (FAILED(m_pMapLoader->Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "OUTER")))
		return E_FAIL;
#endif // TEST_OUTER_MAP

#ifdef TEST_FIRE_EATER_MAP

	//네비 소환
	if (FAILED(m_pMapLoader->Loading_Navigation(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "FIRE_EATER")))
		return E_FAIL;
	if (FAILED(m_pMapLoader->Ready_Nav(TEXT("Layer_Nav"), ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "FIRE_EATER")))
		return E_FAIL;

	if (FAILED(m_pMapLoader->Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "FIRE_EATER")))
		return E_FAIL;
	if (FAILED(m_pMapLoader->Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "FIRE_EATER")))
		return E_FAIL;
#endif // TEST_FIRE_EATER_MAP


#endif // TESTMAP


#ifndef TESTMAP

	if (FAILED(m_pMapLoader->Load_Ready_Nav_All(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION))))
		return E_FAIL;

	if (FAILED(m_pMapLoader->Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), START_MAP)))
		return E_FAIL;
	cout << "[MAP] START_MAP Load 완료" << endl;

	if (FAILED(m_pMapLoader->Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), START_MAP)))
		return E_FAIL;
	cout << "[MAP] START_MAP Ready 완료" << endl;

#endif // !TESTMAP



#pragma endregion

#pragma endregion

	m_fRatio = 1.f;
	Sleep(250); // 안해주면 동기화 안하고 끝나서 안차던데 좋은 방법 있으면 알려주셈

	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_KRAT_HOTEL()
{
	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("셰이더을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));

	//제이슨 파일 안읽고 로드하고 싶은 모델 직접 로드시켜놔야함
	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_HOTEL), TEXT("Prototype_Component_Model_SM_BuildingA_Lift_01"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_BuildingA_Lift_01.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_HOTEL), TEXT("Prototype_Component_Model_SM_BuildingA_Lift_02"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_BuildingA_Lift_02.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(-90.f));

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_HOTEL), TEXT("Prototype_Component_Model_Player"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Player/Player.bin", PreTransformMatrix))))
		return E_FAIL;

	//맵을 생성하기위한 모델 프로토타입을 준비한다.
	/*if (FAILED(Loading_Meshs(ENUM_CLASS(LEVEL::KRAT_HOTEL))))
		return E_FAIL;*/

	lstrcpy(m_szLoadingText, TEXT("네비게이션을(를) 로딩중입니다."));



	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_HOTEL), TEXT("Prototype_GameObject_Player"),
		CPlayer::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_HOTEL), TEXT("Prototype_GameObject_StaticMesh"),
		CStaticMesh::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_HOTEL), TEXT("Prototype_GameObject_StaticMesh_Instance"),
		CStaticMesh_Instance::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;

	return S_OK;
}



HRESULT CLoader::Loading_For_DH()
{
	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("셰이더을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));


	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.004f, 0.004f, 0.004f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_TestMap"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/TestMap.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_OutDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/OutDoor.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_SM_BuildingA_Lift_01"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_BuildingA_Lift_01.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_SM_BuildingA_Lift_02"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_BuildingA_Lift_02.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_SM_BuildingC_Sewer_01"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_BuildingC_Sewer_01.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_SM_Cathedral_FloorBR_03"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_Cathedral_FloorBR_03.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_SM_Station_Floor_01"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_Station_Floor_01.bin", PreTransformMatrix))))
		return E_FAIL;


	/* 상호 작용 문 */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_SM_Station_TrainDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_Station_TrainDoor_01.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_FacotoryDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FacotoryDoor/FacotoryDoor.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix *= XMMatrixRotationY(XMConvertToRadians(-180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_FestivalDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FestivalDoor/FestivalDoor.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_FestivalCrashDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FestivalCrashDoor/FestivalCrashDoor.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.011f, 0.011f, 0.011f);
	PreTransformMatrix *= XMMatrixRotationY(XMConvertToRadians(-180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_StationDoubleDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/StationDoubleDoor/StationDoubleDoor.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	PreTransformMatrix *= XMMatrixRotationY(XMConvertToRadians(-270.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_ShortCutDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/ShortCutDoor/ShortCutDoor.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	PreTransformMatrix *= XMMatrixRotationY(XMConvertToRadians(-90.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_HeavyLock"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/HeavyLock/HeavyLock.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	PreTransformMatrix *= XMMatrixRotationY(XMConvertToRadians(-270.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_HeavyLockSmall"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/HeavyLockSmall/HeavyLockSmall.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(-90.f));

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_FireEater"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FireEater/FireEater.bin", PreTransformMatrix))))
		return E_FAIL;

	//맵을 생성하기위한 모델 프로토타입을 준비한다.
	if (FAILED(m_pMapLoader->Loading_Meshs(ENUM_CLASS(LEVEL::DH), "STATION")))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("네비게이션을(를) 로딩중입니다."));



	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));


	//FIRE EATER로 바꿧어요 (영웅)
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_FireEater"),
		CFuoco::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* 상호 작용 문 */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_SlideDoor"),
		CSlideDoor::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_KeyDoor"),
		CKeyDoor::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_BossDoor"),
		CBossDoor::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_ShortCutDoor"),
		CShortCutDoor::Create(m_pDevice, m_pContext))))
		return E_FAIL;


//#pragma region 맵 오브젝트
//	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_StaticMesh"),
//		CStaticMesh::Create(m_pDevice, m_pContext))))
//		return E_FAIL;
//
//	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_StaticMesh_Instance"),
//		CStaticMesh_Instance::Create(m_pDevice, m_pContext))))
//		return E_FAIL;
//
//	//스태틱 데칼	
//	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_Static_Decal"),
//		CStatic_Decal::Create(m_pDevice, m_pContext))))
//		return E_FAIL;
//
//	//네비게이션 컴포넌트 작동시켜주는 녀석
//	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_Nav"),
//		CNav::Create(m_pDevice, m_pContext))))
//		return E_FAIL;
//
//#pragma endregion
//
//#pragma region 맵 로드
//	string Map = "STATION"; //STATION, TEST
//
//	lstrcpy(m_szLoadingText, TEXT("맵 로딩 중..."));
//	if (FAILED(Load_Map(ENUM_CLASS(LEVEL::DH), Map.c_str())))
//		return E_FAIL;
//
//	lstrcpy(m_szLoadingText, TEXT("맵 생성 중..."));
//	//제이슨으로 저장된 맵을 로드한다.
//	if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::DH), Map.c_str())))
//		return E_FAIL;
//#pragma endregion

#pragma region YW
//스태틱 데칼	
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_Static_Decal"),
		CStatic_Decal::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	//네비게이션 컴포넌트 작동시켜주는 녀석
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_Nav"),
		CNav::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_StaticMesh"),
		CStaticMesh::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_StaticMesh_Instance"),
		CStaticMesh_Instance::Create(m_pDevice, m_pContext))))
		return E_FAIL;

#pragma region 맵 로딩

	m_pGameInstance->ClaerOctoTreeObjects();

	lstrcpy(m_szLoadingText, TEXT("STATION 맵 생성 시작!!..."));
	auto futureStation = async(launch::async, [&]
		{
			if (FAILED(m_pMapLoader->Load_Map(ENUM_CLASS(LEVEL::DH), "STATION")))
				return E_FAIL;
			if (FAILED(m_pMapLoader->Ready_Map(ENUM_CLASS(LEVEL::DH), "STATION")))
				return E_FAIL;

			return S_OK;
		});

	lstrcpy(m_szLoadingText, TEXT("HOTEL 맵 생성 시작!!..."));
	auto futureHotel = async(launch::async, [&]
		{
			if (FAILED(m_pMapLoader->Load_Map(ENUM_CLASS(LEVEL::DH), "HOTEL")))
				return E_FAIL;
			if (FAILED(m_pMapLoader->Ready_Map(ENUM_CLASS(LEVEL::DH), "HOTEL")))
				return E_FAIL;

			return S_OK;
		});

	lstrcpy(m_szLoadingText, TEXT("FIRE_EATER 맵 생성 시작!!..."));
	auto futureFireEater = async(launch::async, [&]
		{
			if (FAILED(m_pMapLoader->Load_Map(ENUM_CLASS(LEVEL::DH), "FIRE_EATER")))
				return E_FAIL;
			if (FAILED(m_pMapLoader->Ready_Map(ENUM_CLASS(LEVEL::DH), "FIRE_EATER")))
				return E_FAIL;

			return S_OK;
		});

	lstrcpy(m_szLoadingText, TEXT("OUTER 맵 생성 시작!!..."));
	auto futureOuter = async(launch::async, [&]
		{
			if (FAILED(m_pMapLoader->Load_Map(ENUM_CLASS(LEVEL::DH), "OUTER")))
				return E_FAIL;
			if (FAILED(m_pMapLoader->Ready_Map(ENUM_CLASS(LEVEL::DH), "OUTER")))
				return E_FAIL;

			return S_OK;
		});

	lstrcpy(m_szLoadingText, TEXT("맵 생성 중..."));

	if (FAILED(futureStation.get()))
		return E_FAIL;
	lstrcpy(m_szLoadingText, TEXT("STATION 맵 생성 완료..."));

	if (FAILED(futureHotel.get()))
		return E_FAIL;
	lstrcpy(m_szLoadingText, TEXT("HOTEL 맵 생성 완료...")); 

	if (FAILED(futureFireEater.get()))
		return E_FAIL;
	lstrcpy(m_szLoadingText, TEXT("FIRE_EATER 맵 생성 완료..."));

	if (FAILED(futureOuter.get()))
		return E_FAIL;
	lstrcpy(m_szLoadingText, TEXT("OUTER 맵 생성 완료..."));

#pragma endregion



	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;
	return S_OK;
}

HRESULT CLoader::Loading_For_JW()
{
	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("셰이더을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));
	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(-90.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_FireEater"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FireEater/FireEater.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Buttler_Train"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Buttler_Train/Buttler_Train.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Buttler_Basic"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Buttler_Basic/Buttler_Basic.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_FestivalLeader"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FestivalLeader/FestivalLeader.bin", PreTransformMatrix))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::JW), TEXT("Prototype_Component_Model_Player"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Player/Player.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::JW), TEXT("Prototype_Component_Model_PlayerWeapon"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Weapon/Bayonet.bin", PreTransformMatrix))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_Buttler_Train_Weapon"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Weapon_Buttler/SK_WP_MOB_ButtlerTrain_01.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::JW), TEXT("Prototype_Component_Model_PlayerLamp"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Lamp.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Elite_Police"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Elite_Police/Elite_Police.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_FireBall"),
		CFireBall::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_Elite_Police_Weapon"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Club/Elite_Police_Weapon.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixScaling(0.004f, 0.004f, 0.004f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::JW), TEXT("Prototype_Component_Model_Train"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Train.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::JW), TEXT("Prototype_Component_Model_Station"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Station.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::JW), TEXT("Prototype_GameObject_PlayerLamp"),
		CPlayerLamp::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_FestivalWeapon"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FestivalLeader/Weapon/FestivalWeapon.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_Elite_Police_Weapon"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Club/Elite_Police_Weapon.bin", PreTransformMatrix))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("네비게이션을(를) 로딩중입니다."));



	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::JW), TEXT("Prototype_GameObject_PlayerWeapon"),
		CBayonet::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::JW), TEXT("Prototype_GameObject_Monster_Buttler_Train"),
		CButtler_Train::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_Weapon"),
		CWeapon_Monster::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::JW), TEXT("Prototype_GameObject_Player"),
		CPlayer::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::JW), TEXT("Prototype_GameObject_PlayerFrontCollider"),
		CPlayerFrontCollider::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Elite_Police"),
		CElite_Police::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_FestivalLeader"),
		CFestivalLeader::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;
	return S_OK;
}

HRESULT CLoader::Loading_For_GL()
{
	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));

	/* For.Prototype_Component_Texture_Button_Hover*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Button_Hover"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Button/Button_Hover.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Button_Highlight"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Button/Line_Highlight.dds")))))
		return E_FAIL;


	/* For.Prototype_Component_Texture_Guide_Background*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Guide_Background"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Guide/Guide_Background.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Sealingwax*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Sealingwax"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Guide/Sealingwax.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_TeamPicture*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_TeamPicture"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Guide/TeamPicture.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_Space*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_Space"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_Space.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_LeftShift"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_LeftShift.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_R"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_R.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_T"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_T.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_G"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_G.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Button_Arrow"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Button/Btn_Arrow.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Bar_Border"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Bar/SketchBorder.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Bar_Background"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Bar/BG_Recollection.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Slot_Background"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Slot/SlotBg_%d.dds"),2))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Weapon_Background"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Weapon/Weapon_Bg.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Cross"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Weapon/Icon_Cross.dds")))))
		return E_FAIL;
	
	m_fRatio = 0.1f;


	lstrcpy(m_szLoadingText, TEXT("셰이더을(를) 로딩중입니다."));

	m_fRatio = 0.3f;
	

	


	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));

	m_fRatio = 0.5f;
	
	lstrcpy(m_szLoadingText, TEXT("네비게이션을(를) 로딩중입니다."));

	m_fRatio = 0.6f;
	
	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));

	m_fRatio = 0.7f;

	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));
	
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Button"),
		CUI_Button::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Text"),
		CUI_Text::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Icon_Item"),
		CIcon_Item::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	m_fRatio = 0.9f;
	

	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_fRatio = 1.f;

	m_isFinished = true;
	return S_OK;
}

#pragma region YW

HRESULT CLoader::Loading_For_YW()
{
	lstrcpy(m_szLoadingText, TEXT("컴포넌트을(를) 로딩중입니다."));
	/* For.Prototype_Component_Transform */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Transform"),
		CTransform::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));

#pragma region 데칼 디폴트 텍스쳐

	/* For.Prototype_Component_Texture_DefaultDecalTexture*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Texture_DefaultDecal"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Decal/T_Decal_Bloodstain_01_N.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_DefaultMaskTexture*/
	/*if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Texture_DefaultMaskTexture"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Decal/T_Decal_01_C_KMH.dds")))))
		return E_FAIL;*/

	/* For.Prototype_Component_Texture_DefaultMaskTexture*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Texture_DefaultMaskTexture"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Decal/T_Decal_12_C_KMH.dds")))))
		return E_FAIL;

#pragma endregion

	lstrcpy(m_szLoadingText, TEXT("셰이더을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));
	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.004f, 0.004f, 0.004f);
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_Train"),
	//	CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Train.bin", PreTransformMatrix))))
	//	return E_FAIL;
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_Station"),
	//	CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Station.bin", PreTransformMatrix))))
	//	return E_FAIL;
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_Hotel"),
	//	CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Hotel.bin", PreTransformMatrix))))
	//	return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_OutDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/OutDoor.bin", PreTransformMatrix))))
		return E_FAIL;

	//아이템 테스트용 모델
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_ErgoItem"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/ErgoItem.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_Stargazer"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Stargazer.bin", PreTransformMatrix))))
		return E_FAIL;

#pragma region  애님 모델
	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(-90.f));

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_Elite_Police"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Elite_Police/Elite_Police.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_WatchDog"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/WatchDog/WatchDog.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_FireEater"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FireEater/FireEater.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_FestivalLeader"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FestivalLeader/FestivalLeader.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_Buttler_Train"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Buttler_Train/Buttler_Train.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_Buttler_Basic"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Buttler_Basic/Buttler_Basic.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_Buttler_Range"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Buttler_Range/Buttler_Range.bin", PreTransformMatrix))))
		return E_FAIL;


#pragma endregion

	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_Buttler_Train_Weapon"),
	//	CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Weapon_Buttler/SK_WP_MOB_ButtlerTrain_01.bin", PreTransformMatrix))))
	//	return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("네비게이션을(를) 로딩중입니다."));

	if (FAILED(m_pMapLoader->Loading_Navigation(ENUM_CLASS(LEVEL::YW), "STATION")))
		return E_FAIL;

	if (FAILED(m_pMapLoader->Loading_Navigation(ENUM_CLASS(LEVEL::YW), "HOTEL")))
		return E_FAIL;

	if (FAILED(m_pMapLoader->Loading_Navigation(ENUM_CLASS(LEVEL::YW), "OUTER")))
		return E_FAIL;

	if (FAILED(m_pMapLoader->Loading_Navigation(ENUM_CLASS(LEVEL::YW), "FIRE_EATER")))
		return E_FAIL;

	if (FAILED(m_pMapLoader->Loading_Navigation(ENUM_CLASS(LEVEL::YW), "TEST")))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_DecalToolObject"),
		CDecalToolObject::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_MapToolObject"),
		CMapToolObject::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_PreviewObject"),
		CPreviewObject::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_MonsterToolObject"),
		CMonsterToolObject::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;
	return S_OK;
}
#pragma endregion


HRESULT CLoader::Loading_For_UI_Texture()
{
	/* For.Prototype_Component_Texture_Button_Hover*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Button_Hover"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Button/Button_Hover.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Button_Highlight"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Button/Line_Highlight.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Select*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Button_Select"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Button/Button_Select.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Guide_Background*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Guide_Background"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Guide/Guide_Background.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Sealingwax*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Sealingwax"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Guide/Sealingwax.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_TeamPicture*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_TeamPicture"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Guide/TeamPicture.dds")))))
		return E_FAIL;

	///* For.Prototype_Component_Texture_TeamPicture*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Tutorial"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Guide/Tutorial_%d.dds"), 10))))
		return E_FAIL;

	///* For.Prototype_Component_Texture_TeamPicture*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Tutorial_BG"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/BackGround/Tutorial_BG.dds")))))
		return E_FAIL;


	/* For.Prototype_Component_Texture_Icon_Key_Space*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_Space"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_Space.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_LeftShift"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_LeftShift.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_LeftCtrl"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_LeftCtrl.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_R"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_R.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_T"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_T.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_G"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_G.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_F"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_F.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_E"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_E.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_C"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_C.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_W"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_W.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_S"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_S.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_A"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_A.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_D"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_D.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_Q"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_Q.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_ESC"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Key_ESC.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_Mouse_Move"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Mouse_Move.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_Mouse_Middle"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Mouse_Middle.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_Mouse_Left"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Mouse_Left.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Icon_Key_Mouse_Right"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Key/Icon_Mouse_Right.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Button_Arrow"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Button/Btn_Arrow.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Bar_Border*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Bar_Border"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Bar/SketchBorder.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Bar_Background*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Bar_Background"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Bar/BG_Recollection.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Bar_Gradation*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Bar_Gradation"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Bar/Gradation.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Bar_Background*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Bar_Gradation_Side"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Bar/SideGra.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Bar_Background*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Bar_Paralyze"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Bar/UIT_Gauge_ParalyzeEnable.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Bar_Gradation*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Gradation_Right"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Global/Gradation_Right.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Bar_Gradation*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Line_Center"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Global/Line_Hor_Cen_Light.dds")))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Bar_Gradation*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Line_Center_Gradation"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Bar/Gradation_Side.dds")))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Lamp"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Slot/Item/Item_Lamp_%d.dds"), 2))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Grinder"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Slot/Item/Item_Grinder.dds")))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Portion"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Slot/Item/Item_Potion_%d.dds"), 4))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Slot_Background"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Slot/SlotBg_%d.dds"), 2))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Slot_Select"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Slot/SlotBg_Select.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Slot_Input"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Slot/SlotBg_Input.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Weapon_Background"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Weapon/Weapon_Bg.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Weapon_Default"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Weapon/Weapon_Default_%d.dds"), 2))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Weapon_Bayonet"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Weapon/Bayonet_%d.dds"), 2))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Weapon_Saber"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Weapon/Saber_%d.dds"), 2))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Weapon_Rapier"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Weapon/Rapier_%d.dds"), 2))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Skill_Type"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/SkillType/FableType_%d.dds"), 3))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Icon_Key_LeftShift*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Direction"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Global/Direction.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Cross"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Weapon/Icon_Cross.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Ergo"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Icon/Egro.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Arm"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/LegionArm/Item_Arm_%d.dds"),1))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Background_Arm"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Background/Bg_HUDline.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Background_Location"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Background/BG_Location.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_LockOn"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Lockon/Lockon.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Fatal"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Lockon/Fatal.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_ActionType"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/SelectActionType/Img_ActionType_%d.dds"),6))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_ActionType_Effect"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/SelectActionType/Img_ActionType_Mask.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Popup_Background"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/BackGround/Popup_Bg_%d.dds"), 3))))
		return E_FAIL;	
	
	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Script_Background"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/BackGround/Script_BG.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Script_AutoEffect"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Script/Flow.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Menu_Select"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/PlayerMenu/MenuIcon_Selected_%d.dds"),2))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Arrow*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_ItemType"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Icon/ItemType_%d.dds"),3))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Line_Vertical"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Icon/Line_Ver_Cen_Light_L.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_ListItem"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Script/ListItem_%d.dds"),2))))
		return E_FAIL;
	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Background_ListItem"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Script/ListItem_BG.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_DieText"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Death/ImgText_Died.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_WatchBG"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Death/Alert_WatchBG.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Watch_Needle"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Death/Alert_Watch_Needle_%d.dds"),2))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Watch_Num"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Death/Alert_Watch_Num_%d.dds"), 4))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Revive_BackGround"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Global/Dissolve_Nonalpha.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_DeBuff"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/DeBuff/Icon_Debuff_%d.dds"),2))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Cloud"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Global/Effect_Cloud.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Location_BackGround"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Location/UIT_TorsionCoil_Bg_Main.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_HotelBtn_BackGround"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Location/UIT_TorsionCoil_HotelBtn_Bg.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_HotelBtn_Deco"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Location/UIT_TorsionCoil_HotelBtn_Deco.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_ChapterOne"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Location/UIT_Chapter_One.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Location"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Location/UIT_Location_%d.dds"),3))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Stat_BackGround"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Stat/CharacterUpgrade_BG_Slot.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Stat_Icon"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Stat/UIT_Icon_Stat_%d.dds"),6))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_EquipList_Icon"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Stat/UIT_ListItem_Weapon_Equip_%d.dds"),2))))
		return E_FAIL;


	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_BarEffect"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Bar/Bg_BrushDot.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Activated"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Text/ImgText_Activated.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_ImgText_Kill"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Text/UIT_ImgText_Kill.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_BeamLightning"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Text/BeamLightning.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Alert_Kill_Deco"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Text/Alert_Kill_ActionDeco.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Button_Highlight*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Alert_Kill_Puppet"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Text/Alert_Kill_Puppet_%d.dds"),2))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::Loading_For_CY()
{
	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("셰이더을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));
	_matrix		PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Model_ToolMeshEffect"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/EffectMesh/SM_Swirl_01_RSW.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Model_SM_BuildingA_Lift_01"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_BuildingA_Lift_01.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Model_SM_BuildingA_Lift_02"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_BuildingA_Lift_02.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Model_SM_BuildingC_Sewer_01"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_BuildingC_Sewer_01.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Model_SM_Cathedral_FloorBR_03"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_Cathedral_FloorBR_03.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixScaling(0.004f, 0.004f, 0.004f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Model_TestMap"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/TestMap.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixScaling(0.004f, 0.004f, 0.004f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Model_Train"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Train.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Model_Station"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Station.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(-90.f));

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Model_Player"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Player/Player.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Model_PlayerWeapon"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Weapon/Bayonet.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Model_PlayerLamp"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Lamp.bin", PreTransformMatrix))))
		return E_FAIL;

	/* For.Prototype_Component_VIBuffer_ToolTrail */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_ToolTrail"),
		CVIBuffer_SwordTrail::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	lstrcpy(m_szLoadingText, TEXT("네비게이션을(를) 로딩중입니다."));



	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));


 	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));
	/* For.Prototype_GameObject_ToolSprite */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolSprite"),
		CToolSprite::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_ToolParticle */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolParticle"),
		CToolParticle::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_ToolMeshEffect */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolMeshEffect"),
		CToolMeshEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_ToolTrailEffect */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolTrailEffect"),
		CToolTrail::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	///* For.Prototype_GameObject_ToolMesh */
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolMesh"),
	//	CDH_ToolMesh::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;


	/* [ 메인 플레이어 로딩 ] */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_Player"),
		CPlayer::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_PlayerWeapon"),
		CBayonet::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_PlayerLamp"),
		CPlayerLamp::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_PlayerFrontCollider"),
		CPlayerFrontCollider::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;
	return S_OK;
}

HRESULT CLoader::Loading_For_YG()
{
	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));

	//땅찍기 데칼
	/* For.Prototype_Component_Texture_FireEater_Slam_Mask*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Texture_FireEater_Slam_Mask"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Decal/T_Decal_01_C_KMH.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_FireEater_Slam_Normal*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Texture_FireEater_Slam_Normal"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Decal/T_Decal_01_N_KMH.dds")))))
		return E_FAIL;

	//스크래치 데칼
	/* For.Prototype_Component_Texture_FireEater_Scratch_Mask*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Texture_FireEater_Scratch_Mask"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Decal/T_Decal_12_C_KMH.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_FireEater_Scratch_Normal*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Texture_FireEater_Scratch_Normal"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Decal/T_Decal_12_N_KMH.dds")))))
		return E_FAIL;

	m_fRatio = 0.1f;

	lstrcpy(m_szLoadingText, TEXT("셰이더을(를) 로딩중입니다."));
	m_fRatio = 0.2f;

	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));

	/* [ 메인 플레이어 로딩 ] */
	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(-90.f));

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Player"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Player/Player.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_PlayerWeapon"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Weapon/Bayonet.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_PlayerLamp"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Lamp.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Elite_Police"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Elite_Police/Elite_Police.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_Elite_Police_Weapon"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Club/Elite_Police_Weapon.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_Wego"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Wego/Wego.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_FireEater"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FireEater/FireEater.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_Buttler_Train_Weapon"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Weapon_Buttler/SK_WP_MOB_ButtlerTrain_01.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_Buttler_Basic_Weapon"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Candle/SK_WP_MOB_Candle_01.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.0115f, 0.0115f, 0.0115f) * XMMatrixRotationY(XMConvertToRadians(-90.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Buttler_Train"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Buttler_Train/Buttler_Train.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Buttler_Basic"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Buttler_Basic/Buttler_Basic.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.004f, 0.004f, 0.004f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Train"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Train.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Station"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Station.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Hotel"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Hotel.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_OutDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/OutDoor.bin", PreTransformMatrix))))
		return E_FAIL;

	/* 상호 작용 문 */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_SM_Station_TrainDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_Station_TrainDoor_01.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_FacotoryDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FacotoryDoor/FacotoryDoor.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix *= XMMatrixRotationY(XMConvertToRadians(-180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_FestivalDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FestivalDoor/FestivalDoor.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_FestivalCrashDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FestivalCrashDoor/FestivalCrashDoor.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.011f, 0.011f, 0.011f);
	PreTransformMatrix *= XMMatrixRotationY(XMConvertToRadians(-180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_StationDoubleDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/StationDoubleDoor/StationDoubleDoor.bin", PreTransformMatrix))))
		return E_FAIL;

	m_fRatio = 0.4f;



	lstrcpy(m_szLoadingText, TEXT("네비게이션을(를) 로딩중입니다."));

	m_fRatio = 0.6f;
	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));

	m_fRatio = 0.7f;
	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));

	/* [ 메인 플레이어 로딩 ] */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_Player"),
		CPlayer::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_PlayerWeapon"),
		CBayonet::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_PlayerLamp"),
		CPlayerLamp::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_PlayerFrontCollider"),
		CPlayerFrontCollider::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_Wego"),
		CWego::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_Elite_Police"),
		CElite_Police::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_FireBall"),
		CFireBall::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_Oil"),
		COil::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_FlameField"),
		CFlameField::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	/* 상호 작용 문 */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_SlideDoor"),
		CSlideDoor::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_KeyDoor"),
		CKeyDoor::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_BossDoor"),
		CBossDoor::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_ShortCutDoor"),
		CShortCutDoor::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	//뒤에 몬스터 붙이는거 뺐어요(영웅) TEXT("Prototype_GameObject_Buttler_Monster_Train") 이거
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_Test"),
		CMonster_Test::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_Buttler_Train"),
		CButtler_Train::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_Buttler_Basic"),
		CButtler_Basic::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	//FIRE EATER로 바꿧어요 (영웅)
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_FireEater"),
		CFuoco::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_Weapon"),
		CWeapon_Monster::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_HPBar"),
		CUI_MonsterHP_Bar::Create(m_pDevice, m_pContext))))
		return E_FAIL;

#pragma region 트리거용

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_TriggerTalk"),
		CTriggerTalk::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_TriggerSound"),
		CTriggerSound::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_TriggerUI"),
		CTriggerUI::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_TriggerBGM"),
		CTriggerBGM::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_TriggerItemLamp"),
		CTriggerItemLamp::Create(m_pDevice, m_pContext))))
		return E_FAIL;
#pragma endregion

#pragma region YW
	//스태틱 데칼	
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_Static_Decal"),
		CStatic_Decal::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	//네비게이션 컴포넌트 작동시켜주는 녀석
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_Nav"),
		CNav::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_StaticMesh"),
		CStaticMesh::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_StaticMesh_Instance"),
		CStaticMesh_Instance::Create(m_pDevice, m_pContext))))
		return E_FAIL;

#pragma region 맵 로딩

	lstrcpy(m_szLoadingText, TEXT("맵 생성 시작!!..."));

	m_pGameInstance->ClaerOctoTreeObjects();

	//if (FAILED(Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "STATION")))
	//	return E_FAIL;
	//if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "STATION")))
	//	return E_FAIL;

	//if (FAILED(Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "HOTEL")))
	//	return E_FAIL;
	//if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "HOTEL")))
	//	return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("STATION 맵 생성 시작!!..."));

	auto futureStation = async(launch::async, [&]
		{
			if (FAILED(m_pMapLoader->Load_Map(ENUM_CLASS(LEVEL::YG), "STATION")))
				return E_FAIL;
			if (FAILED(m_pMapLoader->Ready_Map(ENUM_CLASS(LEVEL::YG), "STATION")))
				return E_FAIL;

			return S_OK;
		});

	lstrcpy(m_szLoadingText, TEXT("HOTEL 맵 생성 시작!!..."));
	auto futureHotel = async(launch::async, [&]
		{
			if (FAILED(m_pMapLoader->Load_Map(ENUM_CLASS(LEVEL::YG), "HOTEL")))
				return E_FAIL;
			if (FAILED(m_pMapLoader->Ready_Map(ENUM_CLASS(LEVEL::YG), "HOTEL")))
				return E_FAIL;

			return S_OK;
		});

	lstrcpy(m_szLoadingText, TEXT("OUTER 맵 생성 시작!!..."));
	auto futureOuter = async(launch::async, [&]
		{
			if (FAILED(m_pMapLoader->Load_Map(ENUM_CLASS(LEVEL::YG), "OUTER")))
				return E_FAIL;
			if (FAILED(m_pMapLoader->Ready_Map(ENUM_CLASS(LEVEL::YG), "OUTER")))
				return E_FAIL;

			return S_OK;
		});

	lstrcpy(m_szLoadingText, TEXT("FIRE_EATER 맵 생성 시작!!..."));
	auto futureFireEater = async(launch::async, [&]
		{
			if (FAILED(m_pMapLoader->Load_Map(ENUM_CLASS(LEVEL::YG), "FIRE_EATER")))
				return E_FAIL;
			if (FAILED(m_pMapLoader->Ready_Map(ENUM_CLASS(LEVEL::YG), "FIRE_EATER")))
				return E_FAIL;

			return S_OK;
		});


	lstrcpy(m_szLoadingText, TEXT("맵 생성 중..."));

	if (FAILED(futureStation.get()))
		return E_FAIL;

	if (FAILED(futureHotel.get()))
		return E_FAIL;

	if (FAILED(futureOuter.get()))
		return E_FAIL;

	if (FAILED(futureFireEater.get()))
		return E_FAIL;

#pragma endregion

#pragma region YG
	///* For.Prototype_GameObject_YGObject */
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGObject"),
	//	CYGObject::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

	///* For.Prototype_GameObject_YGCapsule */
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGCapsule"),
	//	CYGCapsule::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

	///* For.Prototype_GameObject_YGTriangleMesh */
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGTriangleMesh"),
	//	CYGTriangleMesh::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

	///* For.Prototype_GameObject_YGConvexMesh */
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGConvexMesh"),
	//	CYGConvexMesh::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

	///* For.Prototype_GameObject_YGBox */
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGBox"),
	//	CYGBox::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

	///* For.Prototype_GameObject_YGShpere */
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGShpere"),
	//	CYGShpere::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

	///* For.Prototype_GameObject_YGTrrigerWithoutModel */
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGTrrigerWithoutModel"),
	//	CYGTrrigerWithoutModel::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

	///* For.Prototype_GameObject_YGDynamicGib */
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGDynamicGib"),
	//	CYGDynamicGib::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

	///* For.Prototype_GameObject_YGDynamicObj */
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGDynamicObj"),
	//	CYGDynamicObj::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

	///* For.Prototype_GameObject_YGFloor */
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGFloor"),
	//	CYGFloor::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

	///* For.Prototype_GameObject_YGController */
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGController"),
	//	CYGController::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

#pragma endregion

#pragma endregion

	m_fRatio = 1.f;
	Sleep(250); // 안해주면 동기화 안하고 끝나서 안차던데 좋은 방법 있으면 알려주셈

	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;

	return S_OK;
}



CLoader* CLoader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID, atomic<float>& fRatio)
{
	CLoader* pInstance = new CLoader(pDevice, pContext, fRatio);

	if (FAILED(pInstance->Initialize(eNextLevelID)))
	{
		MSG_BOX("Failed to Created : CLoader");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLoader::Free()
{
	__super::Free();

	WaitForSingleObject(m_hThread, INFINITE);

	DeleteObject(m_hThread);

	CloseHandle(m_hThread);

	Safe_Release(m_pGameInstance);

	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);

	CoUninitialize();

	DeleteCriticalSection(&m_CriticalSection);

	Safe_Release(m_pMapLoader);
}
