#include "ParticleEffect.h"
#include "GameInstance.h"

CParticleEffect::CParticleEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffectBase{ pDevice, pContext }
{

}

CParticleEffect::CParticleEffect(const CParticleEffect& Prototype)
	: CEffectBase(Prototype)
{

}

HRESULT CParticleEffect::Initialize_Prototype()
{
	m_KeyFrames.push_back(EFFKEYFRAME{});
	return S_OK;
}

HRESULT CParticleEffect::Initialize(void* pArg)
{
	if (pArg == nullptr)
	{
		MSG_BOX("이펙트는 DESC가 꼭 필요해용");
		return E_FAIL;
	}

	DESC* pDesc = static_cast<DESC*>(pArg);

	m_iNumInstance = pDesc->iNumInstance;
	m_iShaderPass = pDesc->iShaderPass;
	m_ePType =	pDesc->ePType;
	m_iNumInstance = pDesc->iNumInstance;
	m_isLoop =	pDesc->isLoop;
	m_vCenter = pDesc->vCenter;
	m_vLifeTime = pDesc->vLifeTime;
	m_fMaxLifeTime = pDesc->vLifeTime.y;
	m_vPivot = pDesc->vPivot;
	m_vRange =	pDesc->vRange;
	m_vSize = pDesc->vSize;
	m_vSpeed =	pDesc->vSpeed;
	m_bTool = pDesc->bTool;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (!m_bTool)
	{
		json j;
		ifstream ifs(pDesc->pJsonFilePath);

		if (!ifs.is_open())
		{
			return E_FAIL;
		}
		ifs >> j;

		Deserialize(j);
		Ready_Textures_Prototype();
		if (FAILED(Ready_Components(nullptr)))
			return E_FAIL;
	}

	return S_OK;
}

void CParticleEffect::Priority_Update(_float fTimeDelta)
{

}

void CParticleEffect::Update(_float fTimeDelta)
{

	m_pVIBufferCom->Update(fTimeDelta);
	return;
}

void CParticleEffect::Late_Update(_float fTimeDelta)
{
	/* WeightBlend */
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONLIGHT, this);
}

HRESULT CParticleEffect::Render()
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

void CParticleEffect::Set_Loop(_bool isLoop)
{
	m_pVIBufferCom->Set_Loop(isLoop);
}

HRESULT CParticleEffect::Ready_Components(void* pArg)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_ParticleEffect"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Texture_T_SubUV_Explosion_01_8x8_SC_HJS"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	CVIBuffer_Point_Instance::DESC VIBufferDesc = {};
	VIBufferDesc.ePType =		m_ePType;
	VIBufferDesc.iNumInstance = m_iNumInstance;
	VIBufferDesc.isLoop =		m_isLoop;
	VIBufferDesc.vCenter =		m_vCenter;
	VIBufferDesc.vLifeTime =	m_vLifeTime;
	VIBufferDesc.vPivot =		m_vPivot;
	VIBufferDesc.vRange =		m_vRange;
	VIBufferDesc.vSize =		m_vSize;
	VIBufferDesc.vSpeed =		m_vSpeed;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_PointInstance"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), &VIBufferDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CParticleEffect::Bind_ShaderResources()
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

CParticleEffect* CParticleEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CParticleEffect* pInstance = new CParticleEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CParticleEffect::Clone(void* pArg)
{
	CParticleEffect* pInstance = new CParticleEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CParticleEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CParticleEffect::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
}

json CParticleEffect::Serialize()
{
	json j = __super::Serialize();

	j["NumInstance"] = m_iNumInstance;
	j["Range"] = { m_vRange.x, m_vRange.y, m_vRange.z };
	j["Size"] = { m_vSize.x, m_vSize.y };
	j["Center"] = { m_vCenter.x, m_vCenter.y, m_vCenter.z };
	j["Pivot"] = { m_vPivot.x, m_vPivot.y, m_vPivot.z };
	j["LifeTime_Particle"] = { m_vLifeTime.x, m_vLifeTime.y };
	j["MaxLifeTime"] = m_fMaxLifeTime;
	j["Speed"] = { m_vSpeed.x, m_vSpeed.y };
	j["PType"] = m_ePType;
	j["Loop"] = m_isLoop;
	j["Local"] = m_bLocal;
	

	return j;
}

void CParticleEffect::Deserialize(const json& j)
{
	__super::Deserialize(j);

	if (j.contains("NumInstance"))
		m_iNumInstance = j["NumInstance"].get<_uint>();

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

	if (j.contains("MaxLifeTime"))
		m_fMaxLifeTime = j["MaxLifeTime"].get<_float>();

	if (j.contains("Speed") && j["Speed"].is_array() && j["Speed"].size() == 2)
		m_vSpeed = { j["Speed"][0].get<_float>(), j["Speed"][1].get<_float>() };

	if (j.contains("PType"))
		m_ePType = static_cast<PARTICLETYPE>(j["PType"].get<int>());

	if (j.contains("Loop"))
		m_isLoop = j["Loop"].get<_bool>();

	if (j.contains("Local"))
		m_bLocal = j["Local"].get<_bool>();
}
