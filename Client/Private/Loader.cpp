#include "Loader.h"

#include "GameInstance.h"

#include "Camera_Free.h"
#include "Sky.h"
#include "UI_Button.h"


#pragma region LEVEL_KRAT_CENTERAL_STATION
#include "StaticMesh.h"
#include "StaticMesh_Instance.h"
#include "Nav.h"
#include "Static_Decal.h"
#pragma endregion

#pragma region LEVEL_KRAT_HOTEL
#include <future>

#pragma endregion

#pragma region LEVEL_YW
#include "DecalToolObject.h"
#include "MapToolObject.h"
#include "PreviewObject.h"
#include "MonsterToolObject.h"
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
#pragma endregion

#pragma region LEVEL_JW
#include "Oil.h"
#include "Fuoco.h"
#include "FireBall.h"
#include "FlameField.h"
#include "Elite_Police.h"
#include "FestivalLeader.h"
#include "TestAnimObject.h"
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
		return E_FAIL;

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
	//if (FAILED(CEffect_Manager::Get_Instance()->Ready_Effect(TEXT("../Bin/Save/Effect/PE_Player_SkillWeaponParticle.json"))))
	//	return E_FAIL;

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

	/* 상호 작용 문 */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_SM_Station_TrainDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_Station_TrainDoor_01.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixIdentity();
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

	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.011f, 0.011f, 0.011f);
	PreTransformMatrix *= XMMatrixRotationY(XMConvertToRadians(-180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_StationDoubleDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/StationDoubleDoor/StationDoubleDoor.bin", PreTransformMatrix))))
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

#pragma region YW
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

#pragma region 맵 로딩

	lstrcpy(m_szLoadingText, TEXT("맵 생성 시작!!..."));

	m_pGameInstance->ClaerOctoTreeObjects();

	/*if (FAILED(Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "STATION")))
		return E_FAIL;
	if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "STATION")))
		return E_FAIL;

	if (FAILED(Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "HOTEL")))
		return E_FAIL;
	if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "HOTEL")))
		return E_FAIL;

	if (FAILED(Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "OUTER")))
		return E_FAIL;
	if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "OUTER")))
		return E_FAIL;

	if (FAILED(Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "FIRE_EATER")))
		return E_FAIL;
	if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "FIRE_EATER")))
		return E_FAIL;*/

	lstrcpy(m_szLoadingText, TEXT("STATION 맵 생성 시작!!..."));

	auto futureStation = async(launch::async, [&]
		{
			if (FAILED(Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "STATION")))
				return E_FAIL;
			if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "STATION")))
				return E_FAIL;

			return S_OK;
		});

	lstrcpy(m_szLoadingText, TEXT("HOTEL 맵 생성 시작!!..."));
	auto futureHotel = async(launch::async, [&]
		{
			if (FAILED(Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "HOTEL")))
				return E_FAIL;
			if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "HOTEL")))
				return E_FAIL;

			return S_OK;
		});

	lstrcpy(m_szLoadingText, TEXT("OUTER 맵 생성 시작!!..."));
	auto futureOuter = async(launch::async, [&]
		{
			if (FAILED(Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "OUTER")))
				return E_FAIL;
			if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "OUTER")))
				return E_FAIL;

			return S_OK;
		});

	lstrcpy(m_szLoadingText, TEXT("FIRE_EATER 맵 생성 시작!!..."));
	auto futureFireEater = async(launch::async, [&]
		{
			if (FAILED(Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "FIRE_EATER")))
				return E_FAIL;
			if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "FIRE_EATER")))
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

	//맵을 생성하기위한 모델 프로토타입을 준비한다.
	if (FAILED(Loading_Meshs(ENUM_CLASS(LEVEL::DH), "STATION")))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("네비게이션을(를) 로딩중입니다."));



	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));

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
			if (FAILED(Load_Map(ENUM_CLASS(LEVEL::DH), "STATION")))
				return E_FAIL;
			if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::DH), "STATION")))
				return E_FAIL;

			return S_OK;
		});

	lstrcpy(m_szLoadingText, TEXT("HOTEL 맵 생성 시작!!..."));
	auto futureHotel = async(launch::async, [&]
		{
			if (FAILED(Load_Map(ENUM_CLASS(LEVEL::DH), "HOTEL")))
				return E_FAIL;
			if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::DH), "HOTEL")))
				return E_FAIL;

			return S_OK;
		});

	lstrcpy(m_szLoadingText, TEXT("FIRE_EATER 맵 생성 시작!!..."));
	auto futureFireEater = async(launch::async, [&]
		{
			if (FAILED(Load_Map(ENUM_CLASS(LEVEL::DH), "FIRE_EATER")))
				return E_FAIL;
			if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::DH), "FIRE_EATER")))
				return E_FAIL;

			return S_OK;
		});

	lstrcpy(m_szLoadingText, TEXT("OUTER 맵 생성 시작!!..."));
	auto futureOuter = async(launch::async, [&]
		{
			if (FAILED(Load_Map(ENUM_CLASS(LEVEL::DH), "OUTER")))
				return E_FAIL;
			if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::DH), "OUTER")))
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

#pragma region  애님 모델
	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(-90.f));

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_Elite_Police"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Elite_Police/Elite_Police.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_FireEater"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/FireEater/FireEater.bin", PreTransformMatrix))))
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

	if (FAILED(Loading_Navigation(ENUM_CLASS(LEVEL::YW), "STATION")))
		return E_FAIL;

	if (FAILED(Loading_Navigation(ENUM_CLASS(LEVEL::YW), "HOTEL")))
		return E_FAIL;

	if (FAILED(Loading_Navigation(ENUM_CLASS(LEVEL::YW), "OUTER")))
		return E_FAIL;

	if (FAILED(Loading_Navigation(ENUM_CLASS(LEVEL::YW), "FIRE_EATER")))
		return E_FAIL;

	if (FAILED(Loading_Navigation(ENUM_CLASS(LEVEL::YW), "TEST")))
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

HRESULT CLoader::Load_Map(_uint iLevelIndex, const _char* Map)
{
	//맵
	if(FAILED(Loading_Meshs(iLevelIndex, Map)))
		return E_FAIL;

	//네비
	if (FAILED(Loading_Navigation(iLevelIndex, Map)))
		return E_FAIL;
	
	//데칼
	if (FAILED(Loading_Decal_Textures(iLevelIndex, Map)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::Load_Mesh(const wstring& strPrototypeTag, const _char* pModelFilePath, _bool bInstance, _uint iLevelIndex)
{
	_matrix PreTransformMatrix = XMMatrixScaling(PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE);

	// 파일 경로에서 디렉토리, 파일명, 확장자 분리
	wstring wsBasePath = filesystem::path(pModelFilePath).parent_path();
	wstring wsFilename = filesystem::path(pModelFilePath).stem();	// 확장자 뺀 파일명
	wstring wsExtension = filesystem::path(pModelFilePath).extension();

	// LOD0 모델 경로 생성
	string strModelPathLod0 = (filesystem::path(wsBasePath) / filesystem::path(wsFilename + wsExtension)).string();

	if (bInstance == false)
	{
		// LOD1, LOD2 파일명 생성 (_Lod1, _Lod2 붙임)
		wstring wsLod1Name = wsFilename + L"_Lod1" + wsExtension;
		wstring wsLod2Name = wsFilename + L"_Lod2" + wsExtension;

		// LOD1, LOD2 경로 생성
		filesystem::path Lod1Path = filesystem::path(wsBasePath) / wsLod1Name;
		filesystem::path Lod2Path = filesystem::path(wsBasePath) / wsLod2Name;
		string strModelPathLod1 = Lod1Path.string();
		string strModelPathLod2 = Lod2Path.string();

		// LOD0 프로토타입 존재 여부 확인 후 없으면 로드
		if (m_pGameInstance->Find_Prototype(iLevelIndex, strPrototypeTag) == nullptr)
		{
			if (FAILED(m_pGameInstance->Add_Prototype(iLevelIndex, strPrototypeTag,
				CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, strModelPathLod0.c_str(), PreTransformMatrix))))
				return E_FAIL;
		}

		// LOD1 프로토타입 태그 생성 및 로드 (파일 존재하면)
		wstring wsPrototypeTagLod1 = strPrototypeTag + L"_Lod1";
		if (m_pGameInstance->Find_Prototype(iLevelIndex, wsPrototypeTagLod1) == nullptr)
		{
			if (filesystem::exists(strModelPathLod1))
			{
				if (FAILED(m_pGameInstance->Add_Prototype(iLevelIndex, wsPrototypeTagLod1,
					CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, strModelPathLod1.c_str(), PreTransformMatrix))))
					return S_OK; // 실패해도 무시
			}
		}

		// LOD2 프로토타입 태그 생성 및 로드 (파일 존재하면)
		wstring wsPrototypeTagLod2 = strPrototypeTag + L"_Lod2";
		if (m_pGameInstance->Find_Prototype(iLevelIndex, wsPrototypeTagLod2) == nullptr)
		{
			if (filesystem::exists(strModelPathLod2))
			{
				if (FAILED(m_pGameInstance->Add_Prototype(iLevelIndex, wsPrototypeTagLod2,
					CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, strModelPathLod2.c_str(), PreTransformMatrix))))
					return S_OK; // 실패해도 무시
			}
		}

	}
	else
	{
		if (FAILED(m_pGameInstance->Add_Prototype(iLevelIndex, strPrototypeTag,
			CModel_Instance::Create(m_pDevice, m_pContext, MODEL::NONANIM, pModelFilePath, PreTransformMatrix))))
			return E_FAIL;
	}


	return S_OK;
}

HRESULT CLoader::Loading_Meshs(_uint iLevelIndex, const _char* Map)
{

	string ResourcePath = string("../Bin/Save/MapTool/Resource_") + Map + ".json";

	ifstream inFile(ResourcePath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Resource_" + StringToWString(Map) + L".json 파일을 열 수 없습니다: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"에러", MB_OK);

		return S_OK;
	}

	json ResourceJson;
	try
	{
		inFile >> ResourceJson;
		inFile.close();
	}
	catch (const exception& e)
	{
		inFile.close();
		MessageBoxA(nullptr, e.what(), "JSON 파싱 실패", MB_OK);
		return E_FAIL;
	}

	// JSON 데이터 확인
	for (const auto& element : ResourceJson)
	{
		string ModelName = element.value("ModelName", "");
		string Path = element.value("Path", "");

		//갯수도 저장해서 인스턴스용 모델 프로토타입을 만들지 결정해야할듯(충돌여부로 판단하자)
		_uint iObjectCount = element["ObjectCount"];

		_bool bCollision = element["Collision"];

		_bool bNoInstancing = element["NoInstancing"];

		wstring PrototypeTag = {};
		_bool bInstance = false;
		if (bCollision == false && iObjectCount > INSTANCE_THRESHOLD && bNoInstancing == false)
		{
			//인스턴싱용 모델 프로토 타입 생성
			PrototypeTag = L"Prototype_Component_Model_Instance_" + StringToWString(ModelName);
			bInstance = true;

		}
		else
		{
			//모델 프로토 타입 생성
			PrototypeTag = L"Prototype_Component_Model_" + StringToWString(ModelName);
			bInstance = false;
		}


		const _char* pModelFilePath = Path.c_str();

		if (FAILED(Load_Mesh(PrototypeTag, pModelFilePath, bInstance, iLevelIndex)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CLoader::Loading_Navigation(_uint iLevelIndex, const _char* Map/*, _bool bForTool*/)
{
	//bForTool = true;//bForTool 없어질 예정

	wstring wsResourcePath = L"../Bin/Save/NavTool/Nav_" + StringToWString(Map) + L".json";

	// 파일 열기
	ifstream inFile(wsResourcePath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Nav_" + StringToWString(Map) + L".json 파일을 열 수 없습니다.";
		MessageBox(nullptr, ErrorMessage.c_str(), L"에러", MB_OK);
		return S_OK;
	}

	wstring wsPrototypeTag = {};

	//if(bForTool)
		wsPrototypeTag = L"Prototype_Component_Navigation_" + StringToWString(Map);
	//else
	//	wsPrototypeTag = L"Prototype_Component_Navigation";//실제 맵을 위한 로딩(툴과 똑같이 적용할 예정)

	/* Prototype_Component_Navigation */
	if (FAILED(m_pGameInstance->Add_Prototype(
		iLevelIndex,
		wsPrototypeTag.c_str(),
		CNavigation::Create(m_pDevice, m_pContext, wsResourcePath.c_str())
	)))
		return E_FAIL;


	return S_OK;
}
HRESULT CLoader::Loading_Decal_Textures(_uint iLevelIndex, const _char* Map)
{
	string ResourcePath = string("../Bin/Save/DecalTool/Resource_") + Map + ".json";

	ifstream inFile(ResourcePath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Resource_" + StringToWString(Map) + L".json 파일을 열 수 없습니다: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"에러", MB_OK);

		return S_OK;
	}

	json ReadyTextureJson;
	try
	{
		inFile >> ReadyTextureJson;
		inFile.close();
	}
	catch (const exception& e)
	{
		inFile.close();
		MessageBoxA(nullptr, e.what(), "JSON 파싱 실패", MB_OK);
		return E_FAIL;
	}

	//텍스쳐 프로토타입 생성
	for (auto& [PrototypeTag, Path] : ReadyTextureJson.items())
	{
		//이미 있으면 넘기고
		if (m_pGameInstance->Find_Prototype(iLevelIndex, StringToWString(PrototypeTag)) != nullptr)
			continue;

		if (FAILED(m_pGameInstance->Add_Prototype(iLevelIndex, StringToWString(PrototypeTag),
			CTexture::Create(m_pDevice, m_pContext, StringToWString(Path).c_str()))))
			return E_FAIL;
	}

	return S_OK;
}
HRESULT CLoader::Ready_Map(_uint iLevelIndex, const _char* Map)
{
	//어떤 맵을 소환 시킬 것인지?
	if (FAILED(Ready_Meshs(iLevelIndex, Map))) //TEST, STAION
		return E_FAIL;

	//네비 소환
	if (FAILED(Ready_Nav(TEXT("Layer_Nav"), iLevelIndex, Map)))
		return E_FAIL;

	//어떤 데칼을 소환 시킬 것인지?
	if (FAILED(Ready_Static_Decal(iLevelIndex, Map))) //TEST, STATION
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::Ready_Meshs(_uint iLevelIndex, const _char* Map)
{
	string MapPath = string("../Bin/Save/MapTool/Map_") + Map + ".json";

	ifstream inFile(MapPath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Map_" + StringToWString(Map) + L".json 파일을 열 수 없습니다: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"에러", MB_OK);
		return S_OK;
	}

	json MapDataJson;
	inFile >> MapDataJson;
	inFile.close();

	_uint iModelCount = MapDataJson["ModelCount"];
	const json& Models = MapDataJson["Models"];

	for (_uint i = 0; i < iModelCount; ++i)
	{
		string ModelName = Models[i]["ModelName"];
		_uint iObjectCount = Models[i]["ObjectCount"]; //오브젝트 갯수를보고 인스턴싱을 쓸지 말지 결정해야겠다.(아니 충돌여부로 인스턴싱 해야겠다.)
		const json& objects = Models[i]["Objects"];

		_bool bCollision = Models[i]["Collision"];
		_bool bNoInstancing = Models[i]["NoInstancing"];

		//일정 갯수 이상이면 인스턴싱오브젝트로 로드(충돌이 없는 모델이면 인스턴싱)
		if (bCollision == false && iObjectCount > INSTANCE_THRESHOLD && bNoInstancing == false)
		{
			if (FAILED(Ready_StaticMesh_Instance(iObjectCount, objects, ModelName, iLevelIndex, Map)))
				return E_FAIL;
		}
		else
		{
			if (FAILED(Ready_StaticMesh(iObjectCount, objects, ModelName, iLevelIndex, Map)))
				return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CLoader::Ready_StaticMesh(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex, const _char* Map)
{
	for (_uint j = 0; j < iObjectCount; ++j)
	{
#pragma region 월드행렬
		CStaticMesh::STATICMESH_DESC StaticMeshDesc = {};

		const json& WorldMatrixJson = objects[j]["WorldMatrix"];
		_float4x4 WorldMatrix = {};

		for (_int row = 0; row < 4; ++row)
			for (_int col = 0; col < 4; ++col)
				WorldMatrix.m[row][col] = WorldMatrixJson[row][col];

		StaticMeshDesc.WorldMatrix = WorldMatrix;
#pragma endregion

#pragma region 타일링
		//타일링
		if (objects[j].contains("TileDensity"))
		{
			StaticMeshDesc.bUseTiling = true;

			const json& TileDensityJson = objects[j]["TileDensity"];
			StaticMeshDesc.vTileDensity = {
				TileDensityJson[0].get<_float>(),
				TileDensityJson[1].get<_float>()
			};
		}
#pragma endregion

#pragma region 콜라이더
		//콜라이더
		if (objects[j].contains("ColliderType") && objects[j]["ColliderType"].is_number_integer())
		{
			StaticMeshDesc.eColliderType = static_cast<COLLIDER_TYPE>(objects[j]["ColliderType"].get<_int>());
		}
		else
			return E_FAIL;
#pragma endregion

#pragma region 라이트모양
		StaticMeshDesc.iLightShape = objects[j].value("LightShape", 0);
#pragma endregion

		StaticMeshDesc.iRender = 0;
		StaticMeshDesc.iLevelID = iLevelIndex;

		wstring wstrModelName = StringToWString(ModelName);
		wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_");
		ModelPrototypeTag += wstrModelName;

		lstrcpy(StaticMeshDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());

		wstring wsLayerTag = TEXT("Layer_StaticMesh_") + StringToWString(Map); //Layer_StaticMesh_STATION, Layer_StaticMesh_HOTEL

		// 락 걸기
		lock_guard<mutex> lock(m_mtx);

		CGameObject* pGameObject = nullptr;
		if (FAILED(m_pGameInstance->Add_GameObjectReturn(iLevelIndex, TEXT("Prototype_GameObject_StaticMesh"),
			iLevelIndex, wsLayerTag, &pGameObject, &StaticMeshDesc)))
			return E_FAIL;

		m_pGameInstance->PushOctoTreeObjects(pGameObject);
	}

	return S_OK;
}

HRESULT CLoader::Ready_StaticMesh_Instance(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex, const _char* Map)
{
	vector<_float4x4> InstanceMatixs(iObjectCount);

	for (_uint i = 0; i < iObjectCount; ++i)
	{
		const json& WorldMatrixJson = objects[i]["WorldMatrix"];

		for (_int row = 0; row < 4; ++row)
			for (_int col = 0; col < 4; ++col)
				InstanceMatixs[i].m[row][col] = WorldMatrixJson[row][col];
	}


	//오브젝트 생성, 배치
	//LayerTag += StringToWString(ModelName);

	CStaticMesh_Instance::STATICMESHINSTANCE_DESC StaticMeshInstanceDesc = {};
	StaticMeshInstanceDesc.iNumInstance = iObjectCount;//인스턴스 갯수랑
	StaticMeshInstanceDesc.pInstanceMatrixs = &InstanceMatixs;//월드행렬들을 넘겨줘야한다.

	StaticMeshInstanceDesc.iRender = 0;
	StaticMeshInstanceDesc.iLevelID = iLevelIndex;
	//lstrcpy(StaticMeshInstanceDesc.szName, TEXT("SM_TEST_FLOOR"));

	StaticMeshInstanceDesc.iLightShape = objects[0].value("LightShape", 0); //오브젝트중 하나 가져와서 라이트모양을 넣어주자.
	//cout << StaticMeshInstanceDesc.iLightShape << endl;

	wstring wstrModelName = StringToWString(ModelName);
	wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_Instance_"); //인스턴스 용 모델을 준비해야겠는디?
	ModelPrototypeTag += wstrModelName;

	lstrcpy(StaticMeshInstanceDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());

	wstring wsLayerTag = TEXT("Layer_StaticMesh_Instance_") + StringToWString(Map); //Layer_StaticMesh_Instance_STATION, Layer_StaticMesh_Instance_HOTEL
	StaticMeshInstanceDesc.wsMap = StringToWString(Map);

	if (FAILED(m_pGameInstance->Add_GameObject(iLevelIndex, TEXT("Prototype_GameObject_StaticMesh_Instance"),
		iLevelIndex, wsLayerTag, &StaticMeshInstanceDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::Ready_Nav(const _wstring strLayerTag, _uint iLevelIndex, const _char* Map)
{
	CNav::NAV_DESC NavDesc = {};
	NavDesc.iLevelIndex = iLevelIndex;
	NavDesc.wsNavComName = StringToWString(Map);

	if (FAILED(m_pGameInstance->Add_GameObject(iLevelIndex, TEXT("Prototype_GameObject_Nav"),
		iLevelIndex, strLayerTag, &NavDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::Ready_Static_Decal(_uint iLevelIndex, const _char* Map)
{
	//현재 맵에 필요한 데칼 텍스쳐를 로드한다.
	string DecalDataPath = string("../Bin/Save/DecalTool/Decal_") + Map + ".json";
	//string ResourcePath = string("../Bin/Save/MapTool/Resource_") + Map + ".json"; //나중에 쓸듯 맵 바꿀때

	ifstream inFile(DecalDataPath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Decal_" + StringToWString(Map) + L".json 파일을 열 수 없습니다: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"에러", MB_OK);
		return S_OK;
	}

	// JSON 파싱
	json JSON;
	inFile >> JSON;

	// 데이터 만큼 Decal 소환
	for (auto& item : JSON)
	{
		// 4x4 행렬 읽기
		_float4x4 WorldMatrix;
		for (_int iRow = 0; iRow < 4; ++iRow)
		{
			for (_int iCol = 0; iCol < 4; ++iCol)
			{
				WorldMatrix.m[iRow][iCol] = item["WorldMatrix"][iRow][iCol].get<_float>();
			}
		}

		//텍스쳐 프로토타입 이름 전달
		CStatic_Decal::DECAL_DESC Desc = {};
		Desc.WorldMatrix = WorldMatrix;
		Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)] = StringToWString(item["ARMT"].get<string>());
		Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)] = StringToWString(item["N"].get<string>());
		Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)] = StringToWString(item["BC"].get<string>());
		Desc.iLevelID = iLevelIndex;

		// Decal 객체 생성
		if (FAILED(m_pGameInstance->Add_GameObject(iLevelIndex, TEXT("Prototype_GameObject_Static_Decal"),
			iLevelIndex, TEXT("Layer_Decal"), &Desc)))
			return E_FAIL;

	}

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
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Guide/Tutorial_%d.dds"), 5))))
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
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Icon/ItemType_%d.dds"),2))))
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
			if (FAILED(Load_Map(ENUM_CLASS(LEVEL::YG), "STATION")))
				return E_FAIL;
			if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::YG), "STATION")))
				return E_FAIL;

			return S_OK;
		});

	lstrcpy(m_szLoadingText, TEXT("HOTEL 맵 생성 시작!!..."));
	auto futureHotel = async(launch::async, [&]
		{
			if (FAILED(Load_Map(ENUM_CLASS(LEVEL::YG), "HOTEL")))
				return E_FAIL;
			if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::YG), "HOTEL")))
				return E_FAIL;

			return S_OK;
		});

	lstrcpy(m_szLoadingText, TEXT("OUTER 맵 생성 시작!!..."));
	auto futureOuter = async(launch::async, [&]
		{
			if (FAILED(Load_Map(ENUM_CLASS(LEVEL::YG), "OUTER")))
				return E_FAIL;
			if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::YG), "OUTER")))
				return E_FAIL;

			return S_OK;
		});

	lstrcpy(m_szLoadingText, TEXT("FIRE_EATER 맵 생성 시작!!..."));
	auto futureFireEater = async(launch::async, [&]
		{
			if (FAILED(Load_Map(ENUM_CLASS(LEVEL::YG), "FIRE_EATER")))
				return E_FAIL;
			if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::YG), "FIRE_EATER")))
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
}
