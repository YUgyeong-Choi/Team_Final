#include "Loader.h"

#include "GameInstance.h"

#include "Camera_Free.h"
#include "Sky.h"
#include "UI_Button.h"


#pragma region LEVEL_KRAT_CENTERAL_STATION
#include "StaticMesh.h"
#include "StaticMesh_Instance.h"
#include "Nav.h"
#pragma endregion

#pragma region LEVEL_KRAT_HOTEL

#pragma endregion

#pragma region LEVEL_YW
#include "DecalToolObject.h"
#include "MapToolObject.h"
#include "PreviewObject.h"
#pragma endregion

#pragma region LEVEL_CY
#include "ToolSprite.h"
#include "ToolParticle.h"
#include "ToolMeshEffect.h"
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
#pragma endregion

#pragma region LEVEL_DH
#include "PBRMesh.h"
#include "DH_ToolMesh.h"
#include "Player.h"
#include "Bayonet.h"
#pragma endregion

#pragma region LEVEL_GL
#include "UI_Container.h"
#include "Dynamic_UI.h"
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
#include "Ramp.h"
#include "Grinder.h"
#include "Portion.h"
#include "Icon_Weapon.h"
#include "Icon_LegionArm.h"
#include "Monster_Test.h"
#include "Weapon_Monster.h"
#pragma endregion

#pragma region LEVEL_JW
#include "Fuoco.h"
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
	CEffect_Manager::Get_Instance()->Initialize(m_pDevice, m_pContext, TEXT("../Bin/Save/Effect/EffectContainer"));


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

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Icon_LegionArm"),
		CIcon_LegionArm::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Belt"),
		CBelt::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Ramp"),
		CRamp::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Grinder"),
		CGrinder::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Portion"),
		CPortion::Create(m_pDevice, m_pContext))))
		return E_FAIL;



	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	return S_OK;
}

HRESULT CLoader::Loading_For_KRAT_CENTERAL_STATION()
{
	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));


	m_fRatio = 0.1f;

	lstrcpy(m_szLoadingText, TEXT("셰이더을(를) 로딩중입니다."));
	m_fRatio = 0.2f;

	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));

	/* [ 메인 플레이어 로딩 ] */
	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(-90.f));

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_Player"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Player/Player.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_PlayerWeapon"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Weapon/Bayonet.bin", PreTransformMatrix))))
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
	
	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.004f, 0.004f, 0.004f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Train"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Train.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Station"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Station.bin", PreTransformMatrix))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_SM_Station_TrainDoor"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SM_Station_TrainDoor_01.bin", PreTransformMatrix))))
		return E_FAIL;

	m_fRatio = 0.4f;

	//맵을 생성하기위한 모델 프로토타입을 준비한다.
	if (FAILED(Loading_Models(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "STATION")))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("네비게이션을(를) 로딩중입니다."));

	if (FAILED(Loading_Navigation(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "STATION")))
		return E_FAIL;

	m_fRatio = 0.6f;
	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));

	m_fRatio = 0.7f;
	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));

	/* [ 메인 플레이어 로딩 ] */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Player"),
		CPlayer::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PlayerWeapon"),
		CBayonet::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Wego"),
		CWego::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_StaticMesh"),
		CStaticMesh::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_StaticMesh_Instance"),
		CStaticMesh_Instance::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_TestAnimObject"),
		CTestAnimObject::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Monster_Test"),
		CMonster_Test::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Fuoco"),
		CFuoco::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_Weapon"),
		CWeapon_Monster::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	//네비게이션 컴포넌트 작동시켜주는 녀석
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Nav"),
		CNav::Create(m_pDevice, m_pContext))))
		return E_FAIL;

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
	/*if (FAILED(Loading_Models(ENUM_CLASS(LEVEL::KRAT_HOTEL))))
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
	if (FAILED(Loading_Models(ENUM_CLASS(LEVEL::DH), "STATION")))
		return E_FAIL;

	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.004f, 0.004f, 0.004f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_TestMap"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/TestMap.bin", PreTransformMatrix))))
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
	if (FAILED(Loading_Models(ENUM_CLASS(LEVEL::DH), "STATION")))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("네비게이션을(를) 로딩중입니다."));



	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));

#pragma region 맵 오브젝트
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_StaticMesh"),
		CStaticMesh::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_StaticMesh_Instance"),
		CStaticMesh_Instance::Create(m_pDevice, m_pContext))))
		return E_FAIL;

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


	lstrcpy(m_szLoadingText, TEXT("네비게이션을(를) 로딩중입니다."));



	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));

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

	/* For.Prototype_Component_Texture_Button_Select*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Button_Select"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Button/Button_Select_%d.dds"), 3))))
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

	/* For.Prototype_Component_Texture_TeamPicture*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Tutorial"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Guide/Tutorial_%d.dds"),2))))
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

#pragma region 데칼
	/* For.Prototype_Component_VIBuffer_VolumeMesh */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_VIBuffer_VolumeMesh"),
		CVIBuffer_VolumeMesh::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxPos */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Shader_VtxPos"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPos.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements))))
		return E_FAIL;
#pragma endregion

	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));

#pragma region 데칼 테스트 텍스쳐
	/* For.Prototype_Component_Texture_Blood_ARMT*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Texture_Bloodstain_ARMT"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Decal/T_Decal_Bloodstain_01_ARMT.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Blood_N*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Texture_Bloodstain_N"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Decal/T_Decal_Bloodstain_01_N.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Bloodstain_BC*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Texture_Bloodstain_BC"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Decal/T_Decal_Bloodstain_01_BC.dds")))))
		return E_FAIL;

#pragma endregion



	lstrcpy(m_szLoadingText, TEXT("셰이더을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));
	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.004f, 0.004f, 0.004f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_Train"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Train.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_Station"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Station.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(Loading_Models_MapTool(ENUM_CLASS(LEVEL::YW), "STATION")))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("네비게이션을(를) 로딩중입니다."));

	if (FAILED(Loading_Navigation(ENUM_CLASS(LEVEL::YW), "STATION")))
		return E_FAIL;

	if (FAILED(Loading_Navigation(ENUM_CLASS(LEVEL::YW), "HOTEL")))
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

	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;
	return S_OK;
}

HRESULT CLoader::Load_Model(const wstring& strPrototypeTag, const _char* pModelFilePath, _bool bInstance, _uint iLevelIndex)
{
	//이미 프로토타입이존재하는 지확인

	if (m_pGameInstance->Find_Prototype(iLevelIndex, strPrototypeTag) != nullptr)
	{
		//MSG_BOX("이미 프로토타입이 존재함");
		return S_OK;
	}

	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE);

	if (bInstance == false)
	{
		if (FAILED(m_pGameInstance->Add_Prototype(iLevelIndex, strPrototypeTag,
			CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, pModelFilePath, PreTransformMatrix))))
			return E_FAIL;
	}
	else
	{
		if (FAILED(m_pGameInstance->Add_Prototype(iLevelIndex, strPrototypeTag,
			CModel_Instance::Create(m_pDevice, m_pContext, MODEL::NONANIM, pModelFilePath, PreTransformMatrix))))
			return E_FAIL;
	}


	return S_OK;
}

HRESULT CLoader::Loading_Models(_uint iLevelIndex, const _char* Map)
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

		wstring PrototypeTag = {};
		_bool bInstance = false;
		if (bCollision == false /*iObjectCount > INSTANCE_THRESHOLD*/)
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

		if (FAILED(Load_Model(PrototypeTag, pModelFilePath, bInstance, iLevelIndex)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}


HRESULT CLoader::Loading_Models_MapTool(_uint iLevelIndex, const _char* Map)
{
	string ResourcePath = string("../Bin/Save/MapTool/Resource_") + Map + ".json";

	ifstream inFile(ResourcePath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Resource_" + StringToWString(Map) + L".json 파일을 열 수 없습니다: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"에러", MB_OK);

		return S_OK;
	}

	json ReadyModelJson;
	try
	{
		inFile >> ReadyModelJson;
		inFile.close();
	}
	catch (const exception& e)
	{
		inFile.close();
		MessageBoxA(nullptr, e.what(), "JSON 파싱 실패", MB_OK);
		return E_FAIL;
	}

	// JSON 데이터 확인
	for (const auto& element : ReadyModelJson)
	{
		string ModelName = element.value("ModelName", "");
		string Path = element.value("Path", "");

		//모델 프로토 타입 생성
		wstring PrototypeTag = L"Prototype_Component_Model_" + StringToWString(ModelName);

		const _char* pModelFilePath = Path.c_str();

		if (FAILED(Load_Model_MapTool(PrototypeTag, pModelFilePath, iLevelIndex)))
		{
			return E_FAIL;
		}
	}

	return S_OK;

}

HRESULT CLoader::Load_Model_MapTool(const wstring& strPrototypeTag, const _char* pModelFilePath, _uint iLevelIndex)
{
	//이미 프로토타입이존재하는 지확인

	if (m_pGameInstance->Find_Prototype(iLevelIndex, strPrototypeTag) != nullptr)
	{
		//MSG_BOX("이미 프로토타입이 존재함");
		return S_OK;
	}

	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE);

	if (FAILED(m_pGameInstance->Add_Prototype(iLevelIndex, strPrototypeTag,
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, pModelFilePath, PreTransformMatrix))))
		return E_FAIL;

	return S_OK;
}
HRESULT CLoader::Loading_Navigation(_uint iLevelIndex, const _char* Map)
{
	wstring wsResourcePath = L"../Bin/Save/NavTool/Nav_" + StringToWString(Map) + L".json";

	// 파일 열기
	ifstream inFile(wsResourcePath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Nav_" + StringToWString(Map) + L".json 파일을 열 수 없습니다.";
		MessageBox(nullptr, ErrorMessage.c_str(), L"에러", MB_OK);
		return S_OK;
	}

	wstring wsPrototypeTag = L"Prototype_Component_Navigation_" + StringToWString(Map);

	/* Prototype_Component_Navigation */
	if (FAILED(m_pGameInstance->Add_Prototype(
		iLevelIndex,
		wsPrototypeTag.c_str(),
		CNavigation::Create(m_pDevice, m_pContext, wsResourcePath.c_str())
	)))
		return E_FAIL;


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
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Button/Button_Select_%d.dds"), 3))))
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
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Guide/Tutorial_%d.dds"), 2))))
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

	/* For.Prototype_Component_Texture_Bar_Gradation*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Gradation_Right"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Global/Gradation_Right.dds")))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Ramp"),
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

	return S_OK;
}

HRESULT CLoader::Loading_For_CY()
{
	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));

	/* For.Prototype_Component_Texture_T_SubUV_Explosion_01_8x8_SC_HJS */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Texture_T_SubUV_Explosion_01_8x8_SC_HJS"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Effect/SubUV/T_SubUV_Explosion_01_8x8_SC_HJS.dds"), 1))))
		return E_FAIL;


	//_wstring strFileName
	///* For.Prototype_Component_Texture */
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), _wstring("Prototype_Component_Texture_") + ,
	//	CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Effect/T_SubUV_Explosion_01_8x8_SC_HJS.dds"), 1))))
	//	return E_FAIL;


	lstrcpy(m_szLoadingText, TEXT("셰이더을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));
	_matrix		PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Model_ToolMeshEffect"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/EffectMesh/SM_Swirl_01_RSW.bin", PreTransformMatrix))))
		return E_FAIL;

	_matrix		PreTransformMatrixSpot = XMMatrixIdentity();
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

	///* For.Prototype_GameObject_ToolMesh */
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolMesh"),
	//	CDH_ToolMesh::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;
	return S_OK;
}

HRESULT CLoader::Loading_For_YG()
{
	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("셰이더을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));
	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixRotationY(XMConvertToRadians(180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Finoa"),CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM,"../Bin/Resources/Models/TestPhysX/FionaNonAnimBin/Fiona.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.004f, 0.004f, 0.004f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Train"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Train.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Station"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Station.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(-90.f));

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_Player"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Player/Player.bin", PreTransformMatrix))))
		return E_FAIL;


	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(-90.f));

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Wego"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/Bin_Anim/Wego/Wego.bin", PreTransformMatrix))))
		return E_FAIL;

	//PreTransformMatrix = XMMatrixIdentity();
	//PreTransformMatrix = XMMatrixRotationY(XMConvertToRadians(90.f));
 //	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_FinoaAnim"), CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Bin/Resources/Models/TestPhysX/FionaAnimBin/Fiona.bin", PreTransformMatrix))))
	//	return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Floor"), CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/TestPhysX/Floor/Floor.bin"))))
		return E_FAIL;

	for (int i = 1; i <= 11; ++i) {
		wstring protoTag = L"Prototype_Component_Model_Part" + to_wstring(i);
		string modelPath = "../Bin/Resources/Models/TestPhysX/Gib/stone_stage_1_part" + to_string(i) + ".bin";
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), protoTag.c_str(), CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, modelPath.c_str()))))
			return E_FAIL;
	}

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Barrel"), CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/TestPhysX/Barrel/lootbarrel.bin"))))
		return E_FAIL;

	//맵을 생성하기위한 모델 프로토타입을 준비한다.
	if (FAILED(Loading_Models(ENUM_CLASS(LEVEL::YG), "STATION")))
		return E_FAIL;


	lstrcpy(m_szLoadingText, TEXT("네비게이션을(를) 로딩중입니다."));



	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));

	/* For.Prototype_GameObject_YGObject */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGObject"),
		CYGObject::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_YGCapsule */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGCapsule"),
		CYGCapsule::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_YGTriangleMesh */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGTriangleMesh"),
		CYGTriangleMesh::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_YGConvexMesh */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGConvexMesh"),
		CYGConvexMesh::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	
	/* For.Prototype_GameObject_YGBox */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGBox"),
		CYGBox::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_YGShpere */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGShpere"),
		CYGShpere::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_YGTrrigerWithoutModel */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGTrrigerWithoutModel"),
		CYGTrrigerWithoutModel::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_YGDynamicGib */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGDynamicGib"),
		CYGDynamicGib::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_YGDynamicObj */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGDynamicObj"),
		CYGDynamicObj::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_YGFloor */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGFloor"),
		CYGFloor::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_YGController */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGController"),
		CYGController::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* [ 메인 플레이어 로딩 ] */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Player"),
		CPlayer::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_NPCWego */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_NPCWego"),
		CWego::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_StaticMesh"),
		CStaticMesh::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_StaticMesh_Instance"),
		CStaticMesh_Instance::Create(m_pDevice, m_pContext))))
		return E_FAIL;

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
