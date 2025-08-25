#include "ToolParticle.h"

#include "Camera_Manager.h"
#include "GameInstance.h"

CToolParticle::CToolParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CParticleEffect { pDevice, pContext }
{

}

CToolParticle::CToolParticle(const CToolParticle& Prototype)
	: CParticleEffect( Prototype )
{

}

HRESULT CToolParticle::Initialize_Prototype()
{
	m_KeyFrames.push_back(EFFKEYFRAME{});
	return S_OK;
}

HRESULT CToolParticle::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;


	DESC* pDesc = static_cast<DESC*>(pArg);
	m_bLoadingInTool = pDesc->bLoadingInTool;

	if (m_bTool)
	{
		m_tDesc.iNumInstance = pDesc->iNumInstance;
		m_tDesc.ePType = pDesc->ePType;
		m_tDesc.iNumInstance = pDesc->iNumInstance;
		m_tDesc.isLoop = pDesc->isLoop;
		m_tDesc.vCenter = pDesc->vCenter;
		m_tDesc.vLifeTime = pDesc->vLifeTime;
		m_tDesc.vPivot = pDesc->vPivot;
		m_tDesc.vRange = pDesc->vRange;
		m_tDesc.vSize = pDesc->vSize;
		m_tDesc.vSpeed = pDesc->vSpeed;
		m_tDesc.isTileLoop = false;
		m_tDesc.vTileCnt = _float2(static_cast<_float>(pDesc->iTileX), static_cast<_float>(pDesc->iTileY));
		m_tDesc.fTileTickPerSec = m_fTileTickPerSec;
		m_iShaderPass = pDesc->iShaderPass;
		m_fMaxLifeTime = pDesc->vLifeTime.y;
		m_bTool = pDesc->bTool;
	}

	if (m_bLoadingInTool) // 툴 내에서 불러오기 한 경우
	{
		if (FAILED(Ready_Components(nullptr)))
			return E_FAIL;
	}
	else // 새로 생성인 경우
	{
		if (FAILED(Ready_Components(pArg)))
			return E_FAIL;
	}

	for (_int i = 0; i < TU_END; i++)
	{
		m_bTextureUsage[i] = false;
	}
	m_bTextureUsage[TU_DIFFUSE] = true;

	return S_OK;
}

void CToolParticle::Priority_Update(_float fTimeDelta)
{

}

void CToolParticle::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);



	//m_pVIBufferCom->Update(fTimeDelta);
	return;
}

void CToolParticle::Late_Update(_float fTimeDelta)
{
	/* WeightBlend */	

	// 아래 변수로 교체할 것 
	//(RENDERGROUP)m_iRenderGroup;
	// 임시로 고정 지정
	//if (m_iShaderPass != 2)
	//	m_iRenderGroup = (_int)RENDERGROUP::RG_EFFECT_GLOW;
	//else
	//	m_iRenderGroup = (_int)RENDERGROUP::RG_EFFECT_WB;


	m_pGameInstance->Add_RenderGroup((RENDERGROUP)m_iRenderGroup, this);
}

HRESULT CToolParticle::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

 	if (FAILED(m_pShaderCom->Begin(m_iShaderPass)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

		
	return S_OK;
}

void CToolParticle::Update_Tool(_float fTimeDelta, _float fCurFrame)
{
	m_fCurrentTrackPosition = fCurFrame;
	if (m_fCurrentTrackPosition > static_cast<_float>(m_iDuration))
		m_fCurrentTrackPosition = 0;

	Update_Keyframes();

	if (m_bBillboard)
		m_pTransformCom->BillboardToCameraFull(CCamera_Manager::Get_Instance()->GetPureCamPos());

	if (m_bAnimation)
		m_iTileIdx = static_cast<_int>(m_fCurrentTrackPosition);
	else
		m_iTileIdx = 0;

	if (m_iTileX == 0)
		m_iTileX = 1;
	if (m_iTileY == 0)
		m_iTileY = 1;

	m_fTileSize.x = 1.0f / _float(m_iTileX);
	m_fTileSize.y = 1.0f / _float(m_iTileY);
	m_fOffset.x = (m_iTileIdx % m_iTileX) * m_fTileSize.x;
	m_fOffset.y = (m_iTileIdx / m_iTileX) * m_fTileSize.y;

	if (m_pSocketMatrix != nullptr)
	{
		if (m_bFirst)
		{
			m_pVIBufferCom->Set_CombinedMatrix(m_pTransformCom->Get_World4x4());
			m_bFirst = false;
		}
		m_pVIBufferCom->Set_Center(_float3(m_pTransformCom->Get_State(STATE::POSITION).m128_f32[0], m_pTransformCom->Get_State(STATE::POSITION).m128_f32[1], m_pTransformCom->Get_State(STATE::POSITION).m128_f32[2]));
		_vector rotQuat = XMQuaternionRotationMatrix(m_pTransformCom->Get_WorldMatrix());
		_float4 vRot = {};
		XMStoreFloat4(&vRot, rotQuat);
		m_pVIBufferCom->Set_SocketRotation(vRot);
	}
	//else
	//{
	//	m_pVIBufferCom->Set_Center((_float3)(m_pTransformCom->Get_State(STATE::POSITION).m128_f32));
	//}
	//m_pVIBufferCom->Update(fTimeDelta);
	m_pVIBufferCom->Update_Tool(m_fCurrentTrackPosition);
}

HRESULT CToolParticle::Change_InstanceBuffer(void* pArg)
{
	CVIBuffer_Point_Instance::DESC VIBufferDesc = {};
	if (pArg == nullptr) // 툴 내에서 파싱 받아왔을 경우
	{
		m_tDesc.ePType = m_ePType;
		m_tDesc.iNumInstance = m_iNumInstance;
		m_tDesc.isLoop = m_isLoop;
		m_tDesc.isTool = true;
		pArg = &m_tDesc;
	}
	else
	{
		CVIBuffer_Point_Instance::DESC* pDesc = static_cast<CVIBuffer_Point_Instance::DESC*>(pArg);
		m_ePType		= pDesc->ePType;
		m_iNumInstance	= pDesc->iNumInstance;
		m_isLoop		= pDesc->isLoop;
		m_tDesc = *pDesc;
	}

	/* For.Com_VIBuffer */
	if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_PointInstance"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), pArg)))
		return E_FAIL;

	return S_OK;
}

const CVIBuffer_Point_Instance::DESC CToolParticle::Get_InstanceBufferDesc()
{
	return m_tDesc;
}

HRESULT CToolParticle::Ready_Components(void* pArg)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_ParticleEffect"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Texture_T_SubUV_Explosion_01_8x8_SC_HJS"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom[TU_DIFFUSE]))))
		return E_FAIL;

	CVIBuffer_Point_Instance::DESC VIBufferDesc = {};
	if (pArg != nullptr) // 툴에서 새로 생성시
	{
		DESC* pDesc = static_cast<DESC*>(pArg);

		VIBufferDesc.iNumInstance = pDesc->iNumInstance;
		VIBufferDesc.vPivot = pDesc->vPivot;
		VIBufferDesc.vLifeTime = pDesc->vLifeTime;
		VIBufferDesc.vSpeed = pDesc->vSpeed;
		VIBufferDesc.isLoop = pDesc->isLoop;
		VIBufferDesc.ePType = pDesc->ePType;
		VIBufferDesc.vRange = pDesc->vRange;
		VIBufferDesc.vSize = pDesc->vSize;
		VIBufferDesc.vCenter = pDesc->vCenter;
		VIBufferDesc.fTileTickPerSec = m_fTileTickPerSec;

		VIBufferDesc.isTool = true;
		/* For.Com_VIBuffer */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_PointInstance"),
			TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), &VIBufferDesc)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CToolParticle::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_bFlipUV", &m_bFlipUV, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTileSize", &m_fTileSize, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTileOffset", &m_fOffset, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCenterColor", &m_vCenterColor, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fIntensity", &m_fIntensity, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fThreshold", &m_fThreshold, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fTimeAcc, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &m_fEmissiveIntensity, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fStretchFactor", &m_fStretchFactor, sizeof(_float))))
		return E_FAIL;

	if (m_bTextureUsage[TU_DIFFUSE] == true) {
		if (FAILED(m_pTextureCom[TU_DIFFUSE]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0)))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK1] == true) {
		if (FAILED(m_pTextureCom[TU_MASK1]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture1", 0)))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK2] == true) {
		if (FAILED(m_pTextureCom[TU_MASK2]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture2", 0)))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK3] == true) {
		if (FAILED(m_pTextureCom[TU_MASK3]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture3", 0)))
			return E_FAIL;
	}

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShaderCom, "g_DepthTexture")))
		return E_FAIL;

	return S_OK;
}

CToolParticle* CToolParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CToolParticle* pInstance = new CToolParticle(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CToolParticle::Clone(void* pArg)
{
	CToolParticle* pInstance = new CToolParticle(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CToolParticle");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CToolParticle::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
}

json CToolParticle::Serialize()
{
	json j = __super::Serialize();

	j["Range"] = { m_tDesc.vRange.x, m_tDesc.vRange.y, m_tDesc.vRange.z };
	j["Size"] = { m_tDesc.vSize.x, m_tDesc.vSize.y };
	j["Center"] = { m_tDesc.vCenter.x, m_tDesc.vCenter.y, m_tDesc.vCenter.z };
	j["Pivot"] = { m_tDesc.vPivot.x, m_tDesc.vPivot.y, m_tDesc.vPivot.z };
	j["LifeTime_Particle"] = { m_tDesc.vLifeTime.x, m_tDesc.vLifeTime.y };
	j["Speed"] = { m_tDesc.vSpeed.x, m_tDesc.vSpeed.y };

	j["Spin"] = m_tDesc.bSpin;
	j["Orbit"] = m_tDesc.bOrbit;
	j["RotationAxis"] = { m_tDesc.vRotationAxis.x, m_tDesc.vRotationAxis.y, m_tDesc.vRotationAxis.z };
	j["OrbitAxis"] = { m_tDesc.vOrbitAxis.x, m_tDesc.vOrbitAxis.y, m_tDesc.vOrbitAxis.z };
	j["RotationSpeed"] = { m_tDesc.vRotationSpeed.x, m_tDesc.vRotationSpeed.y };
	j["OrbitSpeed"] = { m_tDesc.vOrbitSpeed.x, m_tDesc.vOrbitSpeed.y };

	j["Accel"] = { m_tDesc.vAccel.x, m_tDesc.vAccel.y };
	j["MaxSpeed"] = m_tDesc.fMaxSpeed;
	j["MinSpeed"] = m_tDesc.fMinSpeed;

	j["IsGravity"] = m_tDesc.bGravity;
	j["Gravity"] = m_tDesc.fGravity;
	j["isTileLoop"] = m_tDesc.isTileLoop;

	return j;
}

void CToolParticle::Deserialize(const json& j)
{
	__super::Deserialize(j);

	if (j.contains("Range") && j["Range"].is_array() && j["Range"].size() == 3)
		m_tDesc.vRange = { j["Range"][0].get<_float>(), j["Range"][1].get<_float>(), j["Range"][2].get<_float>() };

	if (j.contains("Size") && j["Size"].is_array() && j["Size"].size() == 2)
		m_tDesc.vSize = { j["Size"][0].get<_float>(), j["Size"][1].get<_float>() };

	if (j.contains("Center") && j["Center"].is_array() && j["Center"].size() == 3)
		m_tDesc.vCenter = { j["Center"][0].get<_float>(), j["Center"][1].get<_float>(), j["Center"][2].get<_float>() };

	if (j.contains("Pivot") && j["Pivot"].is_array() && j["Pivot"].size() == 3)
		m_tDesc.vPivot = { j["Pivot"][0].get<_float>(), j["Pivot"][1].get<_float>(), j["Pivot"][2].get<_float>() };

	if (j.contains("LifeTime_Particle") && j["LifeTime_Particle"].is_array() && j["LifeTime_Particle"].size() == 2)
		m_tDesc.vLifeTime = { j["LifeTime_Particle"][0].get<_float>(), j["LifeTime_Particle"][1].get<_float>() };

	if (j.contains("Speed") && j["Speed"].is_array() && j["Speed"].size() == 2)
		m_tDesc.vSpeed = { j["Speed"][0].get<_float>(), j["Speed"][1].get<_float>() };


	if (j.contains("Spin"))
		m_tDesc.bSpin = j["Spin"].get<_bool>();

	if (j.contains("Orbit"))
		m_tDesc.bOrbit = j["Orbit"].get<_bool>();

	if (j.contains("RotationAxis") && j["RotationAxis"].is_array() && j["RotationAxis"].size() == 3)
		m_tDesc.vRotationAxis = { j["RotationAxis"][0].get<_float>(), j["RotationAxis"][1].get<_float>(), j["RotationAxis"][2].get<_float>() };

	if (j.contains("OrbitAxis") && j["OrbitAxis"].is_array() && j["OrbitAxis"].size() == 3)
		m_tDesc.vOrbitAxis = { j["OrbitAxis"][0].get<_float>(), j["OrbitAxis"][1].get<_float>(), j["OrbitAxis"][2].get<_float>() };

	if (j.contains("RotationSpeed") && j["RotationSpeed"].is_array() && j["RotationSpeed"].size() == 2)
		m_tDesc.vRotationSpeed = { j["RotationSpeed"][0].get<_float>(), j["RotationSpeed"][1].get<_float>() };

	if (j.contains("OrbitSpeed") && j["OrbitSpeed"].is_array() && j["OrbitSpeed"].size() == 2)
		m_tDesc.vOrbitSpeed = { j["OrbitSpeed"][0].get<_float>(), j["OrbitSpeed"][1].get<_float>() };

	if (j.contains("Accel") && j["Accel"].is_array() && j["Accel"].size() == 2)
		m_tDesc.vAccel = { j["Accel"][0].get<_float>(), j["Accel"][1].get<_float>() };

	if (j.contains("MaxSpeed"))
		m_tDesc.fMaxSpeed = j["MaxSpeed"].get<_float>();

	if (j.contains("MinSpeed"))
		m_tDesc.fMinSpeed = j["MinSpeed"].get<_float>();


	if (j.contains("IsGravity"))
		m_tDesc.bGravity = j["IsGravity"].get<_bool>();
	if (j.contains("Gravity"))
		m_tDesc.fGravity = j["Gravity"].get<_float>();
	
	// UV Grid
	if (j.contains("isTileLoop"))
		m_tDesc.isTileLoop = j["isTileLoop"].get<_bool>();
	if (j.contains("TileX"))
		m_tDesc.vTileCnt.x = (_float)(j["TileX"].get<_int>());
	if (j.contains("TileY"))
		m_tDesc.vTileCnt.y = (_float)(j["TileY"].get<_int>());
}
