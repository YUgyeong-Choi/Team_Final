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
		m_iNumInstance = pDesc->iNumInstance;
		m_iShaderPass = pDesc->iShaderPass;
		m_ePType = pDesc->ePType;
		m_iNumInstance = pDesc->iNumInstance;
		m_isLoop = pDesc->isLoop;
		m_vCenter = pDesc->vCenter;
		m_vLifeTime = pDesc->vLifeTime;
		m_fMaxLifeTime = pDesc->vLifeTime.y;
		m_vPivot = pDesc->vPivot;
		m_vRange = pDesc->vRange;
		m_vSize = pDesc->vSize;
		m_vSpeed = pDesc->vSpeed;
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

	return ;
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
	//m_iShaderPass = 2.f;
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

	//m_pVIBufferCom->Update(fTimeDelta);
	m_pVIBufferCom->Update_Tool(m_fCurrentTrackPosition);
}

HRESULT CToolParticle::Change_InstanceBuffer(void* pArg)
{
	CVIBuffer_Point_Instance::DESC VIBufferDesc = {};
	if (pArg == nullptr) // 툴 내에서 파싱 받아왔을 경우
	{
		VIBufferDesc.ePType = m_ePType;
		VIBufferDesc.iNumInstance = m_iNumInstance;
		VIBufferDesc.isLoop = m_isLoop;
		VIBufferDesc.vCenter = m_vCenter;
		VIBufferDesc.vLifeTime = m_vLifeTime;
		VIBufferDesc.vPivot = m_vPivot;
		VIBufferDesc.vRange = m_vRange;
		VIBufferDesc.vSize = m_vSize;
		VIBufferDesc.vSpeed = m_vSpeed;
		VIBufferDesc.isTool = true;
		pArg = &VIBufferDesc;
	}
	else
	{
		CVIBuffer_Point_Instance::DESC* pDesc = static_cast<CVIBuffer_Point_Instance::DESC*>(pArg);
		m_ePType		= pDesc->ePType;
		m_iNumInstance	= pDesc->iNumInstance;
		m_isLoop		= pDesc->isLoop;
		m_vCenter		= pDesc->vCenter;
		m_vLifeTime		= pDesc->vLifeTime;
		m_vPivot		= pDesc->vPivot;
		m_vRange		= pDesc->vRange;
		m_vSize			= pDesc->vSize;
		m_vSpeed		= pDesc->vSpeed;
		m_bGravity		= pDesc->bGravity;
		m_fGravity		= pDesc->fGravity;
	}



	/* For.Com_VIBuffer */
	if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_PointInstance"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), pArg)))
		return E_FAIL;

	return S_OK;
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

		VIBufferDesc.ePType = pDesc->ePType;
		VIBufferDesc.iNumInstance = pDesc->iNumInstance;
		VIBufferDesc.isLoop = pDesc->isLoop;
		VIBufferDesc.vCenter = pDesc->vCenter;
		VIBufferDesc.vLifeTime = pDesc->vLifeTime;
		VIBufferDesc.vPivot = pDesc->vPivot;
		VIBufferDesc.vRange = pDesc->vRange;
		VIBufferDesc.vSize = pDesc->vSize;
		VIBufferDesc.vSpeed = pDesc->vSpeed;
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

	j["Range"] = { m_vRange.x, m_vRange.y, m_vRange.z };
	j["Size"] = { m_vSize.x, m_vSize.y };
	j["Center"] = { m_vCenter.x, m_vCenter.y, m_vCenter.z };
	j["Pivot"] = { m_vPivot.x, m_vPivot.y, m_vPivot.z };
	j["LifeTime_Particle"] = { m_vLifeTime.x, m_vLifeTime.y };
	j["Speed"] = { m_vSpeed.x, m_vSpeed.y };

	return j;
}

void CToolParticle::Deserialize(const json& j)
{
	__super::Deserialize(j);

	if (j.contains("Range") && j["Range"].is_array() && j["Range"].size() == 3)
		m_vRange = { j["Range"][0].get<_float>(), j["Range"][1].get<_float>(), j["Range"][2].get<_float>() };

	if (j.contains("Size") && j["Size"].is_array() && j["Size"].size() == 2)
		m_vSize = { j["Size"][0].get<_float>(), j["Size"][1].get<_float>() };

	if (j.contains("Center") && j["Center"].is_array() && j["Center"].size() == 3)
		m_vCenter = { j["Center"][0].get<_float>(), j["Center"][1].get<_float>(), j["Center"][2].get<_float>() };

	if (j.contains("Pivot") && j["Pivot"].is_array() && j["Pivot"].size() == 3)
		m_vPivot = { j["Pivot"][0].get<_float>(), j["Pivot"][1].get<_float>(), j["Pivot"][2].get<_float>() };

	if (j.contains("LifeTime_Particle") && j["LifeTime_Particle"].is_array() && j["LifeTime_Particle"].size() == 2)
		m_vLifeTime = { j["LifeTime_Particle"][0].get<_float>(), j["LifeTime_Particle"][1].get<_float>() };

	if (j.contains("Speed") && j["Speed"].is_array() && j["Speed"].size() == 2)
		m_vSpeed = { j["Speed"][0].get<_float>(), j["Speed"][1].get<_float>() };

}
