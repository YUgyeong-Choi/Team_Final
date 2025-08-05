#include "Loader.h"

#include "GameInstance.h"

#include "Camera_Free.h"
#include "Sky.h"
#include "UI_Button.h"


#pragma region LEVEL_KRAT_CENTERAL_STATION
#
#pragma endregion

#pragma region LEVEL_KRAT_HOTEL
#include "StaticMesh.h"
#include "StaticMesh_Instance.h"
#pragma endregion

#pragma region LEVEL_YW
#include "MapToolObject.h"
#include "PreviewObject.h"
#pragma endregion

#pragma region LEVEL_CY
#include "ToolSprite.h"
#include "ToolParticle.h"
#include "ToolMeshEffect.h"
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
#pragma endregion

#pragma region LEVEL_DH
#include "StaticMesh.h"
#include "PBRMesh.h"
#include "DH_ToolMesh.h"
#include "Player.h"
#pragma endregion

#pragma region LEVEL_GL
#include "UI_Container.h"
#include "Dynamic_UI.h"
#include "UI_Text.h"
#include "UI_Guide.h"
#include "HP_Bar.h"
#include "Mana_Bar.h"
#include "Stamina_Bar.h"
#pragma endregion

#pragma region LEVEL_JW
#include "TestAnimObject.h"
#pragma endregion

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


	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));

	_matrix PreTransformMatrix = XMMatrixIdentity();
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_SkyBox"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Sky/SkyBox.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
		CPBRMesh::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("셰이더을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Button"),
		CUI_Button::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Text"),
		CUI_Text::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_SpriteEffect */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_SpriteEffect"),
		CSpriteEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_ParticleEffect */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_ParticleEffect"),
		CParticleEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	/* For.Prototype_GameObject_MeshEffect */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_MeshEffect"),
		CMeshEffect::Create(m_pDevice, m_pContext))))
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
	
	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.004f, 0.004f, 0.004f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Train"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Train.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_Station"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/Station.bin", PreTransformMatrix))))
		return E_FAIL;

	m_fRatio = 0.4f;
	lstrcpy(m_szLoadingText, TEXT("네비게이션을(를) 로딩중입니다."));


	m_fRatio = 0.6f;
	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));

	m_fRatio = 0.7f;
	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));

	/* [ 메인 플레이어 로딩 ] */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Player"),
		CPlayer::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_StaticMesh"),
		CStaticMesh::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_TestAnimObject"),
		CTestAnimObject::Create(m_pDevice, m_pContext))))
		return E_FAIL;



	m_fRatio = 1.f;
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
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_DirrectionalLight"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/DirrectionalLight.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_PointLight"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/PointLight.bin", PreTransformMatrix))))
		return E_FAIL;
	_matrix		PreTransformMatrixSpot = XMMatrixIdentity();
	PreTransformMatrixSpot = XMMatrixRotationY(XMConvertToRadians(90.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_Component_Model_SpotLight"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/SpotLight.bin", PreTransformMatrixSpot))))
		return E_FAIL;

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

	lstrcpy(m_szLoadingText, TEXT("네비게이션을(를) 로딩중입니다."));



	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_ToolMesh"),
		CDH_ToolMesh::Create(m_pDevice, m_pContext))))
		return E_FAIL;

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

	///* For.Prototype_Component_Texture_TeamPicture*/
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


	m_fRatio = 0.9f;
	Sleep(250);

	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_fRatio = 1.f;

	m_isFinished = true;
	return S_OK;
}

HRESULT CLoader::Loading_For_YW()
{
	lstrcpy(m_szLoadingText, TEXT("컴포넌트을(를) 로딩중입니다."));
	/* For.Prototype_Component_Transform */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Transform"),
		CTransform::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));


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

	lstrcpy(m_szLoadingText, TEXT("네비게이션을(를) 로딩중입니다."));



	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));

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
