#include "Level_KratCentralStation.h"
#include "GameInstance.h"
#include "Camera_Manager.h"

#include "StaticMesh.h"

CLevel_KratCentralStation::CLevel_KratCentralStation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
	, m_pCamera_Manager{ CCamera_Manager::Get_Instance() }
{

}

HRESULT CLevel_KratCentralStation::Initialize()
{
	if (FAILED(Ready_Lights()))
		return E_FAIL;
	if (FAILED(Ready_Camera()))
		return E_FAIL;
	if (FAILED(Ready_Layer_StaticMesh(TEXT("Layer_StaticMesh"))))
		return E_FAIL;

	return S_OK;
}

void CLevel_KratCentralStation::Update(_float fTimeDelta)
{
	if(m_pGameInstance->Key_Down(DIK_F10)) {
		m_pGameInstance->Set_RenderCollider();
	}

	__super::Update(fTimeDelta);
}

HRESULT CLevel_KratCentralStation::Render()
{
	SetWindowText(g_hWnd, TEXT("게임플레이 레벨입니다."));

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Lights()
{
	LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);	
	LightDesc.vDiffuse = _float4(0.6f, 0.6f, 0.6f, 1.f);
	LightDesc.fAmbient = 0.2f;
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
	
	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;


	/*LightDesc.eType = LIGHT_DESC::TYPE_POINT;
	LightDesc.vPosition = _float4(10.f, 5.0f, 10.f, 1.f);
	LightDesc.fRange = 10.f;
	LightDesc.vDiffuse = _float4(1.f, 0.f, 0.f, 1.f);
	LightDesc.fAmbient = 0.4f;
	LightDesc.vSpecular = _float4(1.f, 0.f, 0.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	LightDesc.eType = LIGHT_DESC::TYPE_POINT;
	LightDesc.vPosition = _float4(15.f, 5.0f, 10.f, 1.f);
	LightDesc.fRange = 10.f;
	LightDesc.vDiffuse = _float4(0.f, 1.f, 0.f, 1.f);
	LightDesc.fAmbient = 0.4f;
	LightDesc.vSpecular = _float4(0.f, 1.f, 0.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;*/


	CShadow::SHADOW_DESC		Desc{};
	Desc.vEye = _float4(0.f, 20.f, -15.f, 1.f);
	Desc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 500.f;
	
	if (FAILED(m_pGameInstance->Ready_Light_For_Shadow(Desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Camera()
{
	m_pCamera_Manager->Initialize(LEVEL::STATIC);
	m_pCamera_Manager->SetFreeCam();

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Layer_StaticMesh(const _wstring strLayerTag)
{
	CStaticMesh::STATICMESH_DESC Desc{};
	Desc.iRender = 0;
	Desc.m_eLevelID = LEVEL::KRAT_CENTERAL_STATION;
	Desc.m_vInitPos = _float3(0.f, 0.f, 0.f);
	Desc.m_vInitScale = _float3(1.f, 1.f, 1.f);
	Desc.szMeshID = TEXT("SM_OldTown_BrickFloor_01");
	lstrcpy(Desc.szName, TEXT("SM_TEST_FLOOR"));
	Desc.szShaderID = TEXT("VtxMesh");

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_LevelStaticMesh"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), strLayerTag, &Desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Layer_Sky(const _wstring strLayerTag)
{
	return S_OK;
}

CLevel_KratCentralStation* CLevel_KratCentralStation::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_KratCentralStation* pInstance = new CLevel_KratCentralStation(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_KratCentralStation");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_KratCentralStation::Free()
{
	__super::Free();

}
