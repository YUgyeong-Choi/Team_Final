#include "MainApp.h"
#include "GameInstance.h"
#include "Level_Loading.h"
#include "Camera_Manager.h"
#include "Effect_Manager.h"
#include "LockOn_Manager.h"
#include "UI_Manager.h"

#include "Static_UI.h"
#include "Dynamic_UI.h"
#include "UI_Video.h"
#include "UI_Text.h"

#include "UI_Feature_UV.h"
#include "UI_Feature_Fade.h"
#include "UI_Feature_Position.h"
#include "UI_Featrue_Scale.h"
#include "UI_Feature_Rotation.h"
#include "UI_Feature_Color.h"
#include "UI_Bar_Loading.h"

#include "Sky.h"
CMainApp::CMainApp()
	: m_pGameInstance { CGameInstance::Get_Instance() }
{

	Safe_AddRef(m_pGameInstance);
}

#ifdef _DEBUG
void EnableConsole()
{
	AllocConsole();

	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONIN$", "r", stdin);
	freopen_s(&fp, "CONOUT$", "w", stderr);

	SetConsoleTitle(L"Debug Console");
}
#endif

HRESULT CMainApp::Initialize()
{
#ifdef _DEBUG
	EnableConsole(); // 콘솔 창 띄우기
#endif
	ENGINE_DESC			EngineDesc{};

	EngineDesc.hInstance = g_hInst;
	EngineDesc.hWnd = g_hWnd;
	EngineDesc.iWinSizeX = g_iWinSizeX;
	EngineDesc.iWinSizeY = g_iWinSizeY;
	EngineDesc.isWindowed = true;
	EngineDesc.iNumLevels = static_cast<_uint>(LEVEL::END);

	if (FAILED(m_pGameInstance->Initialize_Engine(EngineDesc, &m_pDevice, &m_pContext)))
		return E_FAIL;

	if (FAILED(Ready_Fonts()))
		return E_FAIL;

	if (FAILED(Ready_Prototype_Component()))
		return E_FAIL;

	if (FAILED(Start_Level(LEVEL::LOGO)))
		return E_FAIL;

	if (FAILED(Ready_Static()))
		return E_FAIL;

	if (FAILED(Ready_Sound()))
		return E_FAIL;

	CUI_Manager::Get_Instance()->Initialize();
	CCamera_Manager::Get_Instance()->Initialize(LEVEL::STATIC);

    return S_OK;
}

void CMainApp::Update(_float fTimeDelta)
{
//#ifdef _DEBUG
	m_fTimeAcc += fTimeDelta;
//#endif

	m_pGameInstance->Update_Engine(fTimeDelta);
}

HRESULT CMainApp::Render()
{
	m_pGameInstance->Begin_Draw();

	m_pGameInstance->Draw();

//#ifdef _DEBUG
	++m_iRenderCount;

	if (m_fTimeAcc >= 1.f)
	{
		wsprintf(m_szFPS, TEXT("FPS:%d"), m_iRenderCount);
		m_fTimeAcc = 0.f;
		m_iRenderCount = 0;
	}

	m_pGameInstance->Draw_Font(TEXT("Font_151"), m_szFPS, _float2(0.f, 0.f), XMVectorSet(1.f, 1.f, 0.f, 1.f));
//#endif


	m_pGameInstance->End_Draw();



    return S_OK;
}

HRESULT CMainApp::Ready_Fonts()
{
	if (FAILED(m_pGameInstance->Add_Font(TEXT("Font_151"), TEXT("../Bin/Resources/Fonts/151ex.spritefont"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Font(TEXT("Font_Bold"), TEXT("../Bin/Resources/Fonts/Title_Bold.spritefont"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Font(TEXT("Font_Medium"), TEXT("../Bin/Resources/Fonts/Info_Medium.spritefont"))))
		return E_FAIL;

	return S_OK;
}

HRESULT CMainApp::Ready_Prototype_Component()
{
	//트랜스폼 여러개 만들고 싶어서 만듬(영웅)
	/* For.Prototype_Component_Transform */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Transform"),
		CTransform::Create(m_pDevice, m_pContext))))
		return E_FAIL;

#pragma region 데칼
	/* For.Prototype_Component_VIBuffer_VolumeMesh */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_VolumeMesh"),
		CVIBuffer_VolumeMesh::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxPos */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Decal"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Decal.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements))))
		return E_FAIL;
#pragma endregion

	/* For.Prototype_Component_VIBuffer_Rect*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
		CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_Component_VIBuffer_Cube */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Cube"),
		CVIBuffer_Cube::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	
	/* For.Prototype_Component_VIBuffer_PointInstance */
	CVIBuffer_Point_Instance::DESC PIDesc = {};
	PIDesc.isTool = true;
	// 클라이언트용은 Tag변경해서 Prototype 수업코드처럼 별개 생성 
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_PointInstance"),
		CVIBuffer_Point_Instance::Create(m_pDevice, m_pContext, &PIDesc))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxPosTex */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxPosTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxNorTex */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxNorTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxNorTex.hlsl"), VTXNORTEX::Elements, VTXNORTEX::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxMesh */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxMesh_Instanve */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh_Instance"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxMesh_Instance.hlsl"), VTXMESH_INSTANCE::Elements, VTXMESH_INSTANCE::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxPBRMesh */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxPBRMesh"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPBRMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxAnimMesh */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl"), VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxCube */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxCube"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxCube.hlsl"), VTXCUBE::Elements, VTXCUBE::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_UI */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_UI"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_DynamicUI */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_DynamicUI"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_DynamicUI.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_SpriteEffect */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SpriteEffect"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_SpriteEffect.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		return E_FAIL;
	
	/* For.Prototype_Component_Shader_MeshEffect */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_MeshEffect"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_MeshEffect.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;
	
	/* For.Prototype_Component_Shader_ParticleEffect */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_ParticleEffect"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_ParticleEffect.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements))))
		return E_FAIL;
	
	/* For.Prototype_Component_Shader_TrailEffect */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_TrailEffect"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_TrailEffect.hlsl"), VTXPOS_TRAIL::Elements, VTXPOS_TRAIL::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_WaterPuddle */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_WaterPuddle"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_WaterPuddle.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;


	/* For.Prototype_Component_PhysX_Static */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Static"), CPhysXStaticActor::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_Component_PhysX_Dynamic */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), CPhysXDynamicActor::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_Component_PhysX_Controller */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Controller"), CPhysXController::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	return S_OK;
}

HRESULT CMainApp::Ready_Static()
{
	/* [ 볼륨메트릭 노이즈텍스쳐 ] */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_perlin_volume"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/perlin_volume.dds")))))
		return E_FAIL;

	/* For.Prototype_GameObject_Sky */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Sky"),
		CSky::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* [ 카메라 소환 ] */
	/* For.Prototype_GameObject_Camera_Free */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Camera_Free"),
		CCamera_Free::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_Camera_Orbital */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Camera_Orbital"),
		CCamera_Orbital::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_Camera_Orbital */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Camera_CutScene"),
		CCamera_CutScene::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_DefaultARM"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/ARM_Default.png")))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Emissive"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Emissive.dds")))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_NoiseMap"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Noise.png")))))
		return E_FAIL;
	/* [ 보스문 디졸브 마스크 ] */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_NoiseMap2"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Effect/T_Tile_Noise_39_C_RSW.dds")))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_FireElement"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Effect/T_Tile_Noise_57_C_HJS_D.dds")))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_FireElementMask"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Effect/T_Tile_Noise_56_C_HJS.dds")))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_FireElementMask2"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Effect/T_Tile_Noise_02_C_LGS.dds")))))
		return E_FAIL;

	return S_OK;
}

HRESULT CMainApp::Ready_Sound()
{
	/* [ 사운드 생성 ] (is3D / isLoop / isStreaming) */
	m_pGameInstance->LoadSound("../Bin/Resources/Sound/BGM/", false, true);
	m_pGameInstance->LoadSound("../Bin/Resources/Sound/BGMIntro/", false, false);
	m_pGameInstance->LoadSound("../Bin/Resources/Sound/SoundSpace/", false, true);
	//ADD_SOUND_EX(BGM, "../Bin/Resources/Sound/BGM/", false, true, false);
	//ADD_SOUND_EX(BGMIntro, "../Bin/Resources/Sound/BGMIntro/", false, false, false);
	//ADD_SOUND_EX(SoundSpace, "../Bin/Resources/Sound/SoundSpace/", false, true, false);

	/* [ UI용 사운드 ] */
	ADD_SOUND_EX(UI, "../Bin/Resources/Sound/UI/", false, false, false);

	/* [ 컷씬때 추가로 필요한 사운드 ] */
	ADD_SOUND_EX(CutSceneExtra, "../Bin/Resources/Sound/CutScene/Camera/", false, false);

	/* [ 플레이어 사운드 ] */
	ADD_SOUND_EX(Player, "../Bin/Resources/Sound/Unit/Player/", false, false, false);
	ADD_SOUND_EX(Grinder, "../Bin/Resources/Sound/Unit/Player/Grinder/", false, false, false);
	ADD_SOUND_EX(Weapon, "../Bin/Resources/Sound/Unit/Weapon/", false, false, false);

	/* [ 동영상용 사운드 ] */
	ADD_SOUND_EX(Intro, "../Bin/Resources/Sound/Video/Intro/", false, false, false);

	///* [ 컷씬 문용 사운드 ] */
	ADD_SOUND_EX(CutSceneDoor, "../Bin/Resources/Sound/CutScene/Door/", false, false, false);

	/* [ 번개 사운드 ] */
	ADD_SOUND_EX(Thunder, "../Bin/Resources/Sound/Thunder/", false, false, false);

	/* [ 스테이션 사운드 ] */
	ADD_SOUND_EX(Station, "../Bin/Resources/Sound/Station/", true, false, false);

	/* [ 트리거용 사운드 ] */
	ADD_SOUND_EX(Trigger, "../Bin/Resources/Sound/Trigger/", false, false, false);


	ADD_SOUND_EX(FireEater, "../Bin/Resources/Sound/Unit/FireEater/", true, false, false);
	ADD_SOUND_EX(FestivalLeader, "../Bin/Resources/Sound/Unit/FestivalLeader/", true, false, false);
	ADD_SOUND_EX(Police, "../Bin/Resources/Sound/Unit/Police/", true, false, false);

	// 램프 줍기 전 사운드
	ADD_SOUND_EX(LampAmb, "../Bin/Resources/Sound/LampAmb/", true, false, false);

	/* [ Breakable 부서지는 사운드 ] */ 
	ADD_SOUND_EX(Breakable, "../Bin/Resources/Sound/Breakable/", true, false, false);

	ADD_SOUND_EX(Stargazer, "../Bin/Resources/Sound/Stargazer/", true, false, false);

	/* [ 숏컷 사운드 ] */ 
	//ADD_SOUND_EX(ShortCutDoor, "../Bin/Resources/Sound/ShortCutDoor/", false, false, false);

	/* [ 일반 몬스터 사운드 ] */
	ADD_SOUND_EX(Buttler, "../Bin/Resources/Sound/Unit/Buttler/", true, false, false);
	ADD_SOUND_EX(Watchdog, "../Bin/Resources/Sound/Unit/Watchdog/", true, false, false);

	return S_OK;
}

HRESULT CMainApp::Start_Level(LEVEL eStartLevel)
{
	if (FAILED(Ready_Loading()))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, eStartLevel))))
		return E_FAIL;

	return S_OK;
}

HRESULT CMainApp::Ready_Loading()
{
	
	/* For.Prototype_Component_Texture_BackGround_Loading_Desk*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_BackGround_Loading_Desk"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Loading/Loading_DeskBg.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_BackGround_Loading_Desk*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_BackGround_Loading_Photo"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Loading/Loading_PhotoBg.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_BackGround_Loading_Desk*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_BackGround_Loading_Tip"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Loading/Loading_TipBg.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_BackGround_Loading_Paper*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_BackGround_Loading_Paper"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Loading/Loading_Paper.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Loading_Object*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Loading_Object"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Loading/Object/Loading_Object_%d.dds"), 6))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Loading_Image*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Loading_Image"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Loading/PhotoImage/PhotoImage.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Img_ChGear*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Img_ChGear"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Loading/gauge/Img_ChGear.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Img_ChNose*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Img_ChNose"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Loading/gauge/Img_ChNose.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Img_ChNose*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Img_ChHead"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/UI/Loading/gauge/Img_ChHead.dds")))))
		return E_FAIL;

	/* For.Prototype_Component_Texture_Loading_Image*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Dissolve"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Global/Dissolve.dds")))))
		return E_FAIL;


	/* For.Prototype_Component_UI_Feature_UV*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_UI_Feature_UV"),
		CUI_Feature_UV::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_Component_UI_Feature_Fade*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_UI_Feature_Fade"),
		CUI_Feature_Fade::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_Component_UI_Feature_Fade*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_UI_Feature_Pos"),
		CUI_Feature_Position::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_Component_UI_Feature_Scale*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_UI_Feature_Scale"),
		CUI_Feature_Scale::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_Component_UI_Feature_Scale*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_UI_Feature_Rotation"),
		CUI_Feature_Rotation::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_Component_UI_Feature_Scale*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_UI_Feature_Color"),
		CUI_Feature_Color::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	/* For.Prototype_GameObject_Static_UI */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Static_UI"),
		CStatic_UI::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_Dynamic_UI */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Dynamic_UI"),
		CDynamic_UI::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_Video_UI */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Video"),
		CUI_Video::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_Video_UI */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Text"),
		CUI_Text::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_Video_UI */
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Bar_Loading"),
		CUI_Bar_Loading::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	 
	return S_OK;
}


CMainApp* CMainApp::Create()
{
	CMainApp* pInstance = new CMainApp();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMainApp::Free()
{
    __super::Free();

	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);

	/* [ 싱글톤 삭제 ] */
	CUI_Manager::Destroy_Instance();
	CCamera_Manager::Destroy_Instance();
	CEffect_Manager::Destroy_Instance();
	CLockOn_Manager::Destroy_Instance();
	

	m_pGameInstance->Release_Engine();

	Safe_Release(m_pGameInstance);
}
