#include "DH_ToolMesh.h"
#include "GameInstance.h"

CDH_ToolMesh::CDH_ToolMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CDH_ToolMesh::CDH_ToolMesh(const CDH_ToolMesh& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CDH_ToolMesh::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CDH_ToolMesh::Initialize(void* pArg)
{
	CDH_ToolMesh::DHTOOL_DESC* PBRMeshDESC = static_cast<DHTOOL_DESC*>(pArg);
	m_szMeshID = PBRMeshDESC->szMeshID;
	m_iID = PBRMeshDESC->iID;
	m_eTargetLevel = PBRMeshDESC->eLEVEL;
	m_InitPos = PBRMeshDESC->m_vInitPos;

	PBRMeshDESC->fSpeedPerSec = 0.f;
	PBRMeshDESC->fRotationPerSec = 0.f;

	if (FAILED(__super::Initialize(PBRMeshDESC)))
		return E_FAIL;
	
	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Light()))
		return E_FAIL;

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(m_InitPos.x, m_InitPos.y, m_InitPos.z, 1.f));
	m_pTransformCom->SetUp_Scale(0.35f, 0.35f, 0.35f);

	return S_OK;
}

void CDH_ToolMesh::Priority_Update(_float fTimeDelta)
{
	if (m_bDead)
		m_pGameInstance->Remove_Light(ENUM_CLASS(LEVEL::DH), m_pLight);

	/* [ 플레이어 찾기 ] */
	if (!m_pPlayer)
		m_pPlayer = m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player"));

	/* [ 플레이어와 거리 측정 ] */
	if (m_pPlayer)
	{
		_vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
		_vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);

		_vector vDiff = XMVectorSubtract(vPlayerPos, vMyPos);
		vDiff = XMVectorSetY(vDiff, 0.f);

		//제곱 연산 최적화
		_float fDistSq = XMVectorGetX(XMVector3LengthSq(vDiff));
		const _float fTh = m_fPlayerFar;
		const _float fThSq = fTh * fTh;

		if (fDistSq > fThSq)
			SetIsPlayerFar(true);
		else
			SetIsPlayerFar(false);
	}

	if (m_bLightOnOff)
		m_pLight->Get_LightDesc()->bIsUse = true;
	else
		m_pLight->Get_LightDesc()->bIsUse = false;
}

void CDH_ToolMesh::Update(_float fTimeDelta)
{
	_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
	XMStoreFloat4(&m_pLight->Get_LightDesc()->vPosition, vPosition);

	_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
	XMStoreFloat4(&m_pLight->Get_LightDesc()->vDirection, XMVectorSetW(vLook, 0.f));
}

void CDH_ToolMesh::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONLIGHT, this);
}

HRESULT CDH_ToolMesh::Render()
{
	if (m_bLightDebug == false)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	/* [ 픽킹 아이디 넘기기 ] */
	_float fID = static_cast<_float>(m_iID);
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fID", &fID, sizeof(_float))))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;

		m_pShaderCom->Begin(1);

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CDH_ToolMesh::Ready_Components(void* pArg)
{
	/* Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxPBRMesh")),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Model_")) + m_szMeshID,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CDH_ToolMesh::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

HRESULT CDH_ToolMesh::Ready_Light()
{
	/* [ 우선 툴 레벨에서 작동이 가능해야한다. ] */
	//1. 라이트 모델은 하나의 라이트만 가지고 있는다.
	//2. 라이트는 라이트 모델의 움직임을 계속 따라간다.
	//3. 라이트 모델은 툴 레벨에 전부 Push_back 으로 저장된다.
	//4. 제이슨 파일로 저장된다.

	LIGHT_DESC			LightDesc{};

	if(m_szMeshID == TEXT("PointLight"))
	{
		LightDesc.eType = LIGHT_DESC::TYPE_POINT;
		LightDesc.vPosition = _float4(10.f, 5.0f, 10.f, 1.f);

		LightDesc.fAmbient = 0.2f;
		LightDesc.fIntensity = 1.f;
		LightDesc.fRange = 10.f;
		LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
		LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
		LightDesc.fFogDensity = 0.f;
		LightDesc.fFogCutoff = 15.f;
		LightDesc.bIsVolumetric = true;
		LightDesc.bIsPlayerFar = false;
		LightDesc.bIsUse = true;
	}
	if (m_szMeshID == TEXT("SpotLight"))
	{
		LightDesc.eType = LIGHT_DESC::TYPE_SPOT;
		LightDesc.vPosition = _float4(10.f, 5.0f, 10.f, 1.f);
		LightDesc.fFalloff = 1.f;
		LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
		LightDesc.fInnerCosAngle = cosf(XMConvertToRadians(30.f));
		LightDesc.fOuterCosAngle = cosf(XMConvertToRadians(45.f));
		LightDesc.fAmbient = 0.2f;
		LightDesc.fIntensity = 1.f;
		LightDesc.fRange = 10.f;
		LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
		LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
		LightDesc.fFogDensity = 0.f;
		LightDesc.fFogCutoff = 15.f;
		LightDesc.bIsVolumetric = true;
		LightDesc.bIsPlayerFar = false;
		LightDesc.bIsUse = true;

	}
	if (m_szMeshID == TEXT("DirrectionalLight"))
	{
		LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
		LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);

		LightDesc.fAmbient = 0.2f;
		LightDesc.fIntensity = 1.f;
		LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
		LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
		LightDesc.fFogDensity = 0.f;
		LightDesc.bIsVolumetric = true;
		LightDesc.bIsPlayerFar = false;
		LightDesc.bIsUse = true;
	}

	m_eTargetLevel = static_cast<LEVEL>(m_pGameInstance->GetCurrentLevelIndex());
	//m_eTargetLevel = LEVEL::DH;
	//m_bLightDebug = true;
	if (FAILED(m_pGameInstance->Add_LevelLightDataReturn(ENUM_CLASS(m_eTargetLevel), LightDesc, &m_pLight)))
		return E_FAIL;

	return S_OK;
}

CDH_ToolMesh* CDH_ToolMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDH_ToolMesh* pGameInstance = new CDH_ToolMesh(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CDH_ToolMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CDH_ToolMesh::Clone(void* pArg)
{
	CDH_ToolMesh* pGameInstance = new CDH_ToolMesh(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CDH_ToolMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CDH_ToolMesh::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
