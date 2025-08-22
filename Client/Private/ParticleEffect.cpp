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

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;


	m_eEffectType = EFF_PARTICLE;
	m_bFirst = true;
	return S_OK;
}

void CParticleEffect::Priority_Update(_float fTimeDelta)
{

}

void CParticleEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (m_pSocketMatrix != nullptr)
	{
		if (m_bFirst)
		{
			m_pVIBufferCom->Set_CombinedMatrix(m_CombinedWorldMatrix);
			m_bFirst = false;
		}
		m_pVIBufferCom->Set_Center((_float3)(m_CombinedWorldMatrix.m[3]));
		XMMATRIX socketWorld = XMLoadFloat4x4(m_pSocketMatrix);
		XMVECTOR rotQuat = XMQuaternionRotationMatrix(socketWorld);
		_float4 vRot = {};
		XMStoreFloat4(&vRot, rotQuat);
		m_pVIBufferCom->Set_SocketRotation(vRot);
	}
	else
	{
		m_pVIBufferCom->Set_Center((_float3)(m_pTransformCom->Get_State(STATE::POSITION).m128_f32));
	}

	m_pVIBufferCom->Update(fTimeDelta);
	return; 
}

void CParticleEffect::Late_Update(_float fTimeDelta)
{
	/* WeightBlend */
	//m_iRenderGroup = (_int)RENDERGROUP::RG_EFFECT_GLOW;

	m_pGameInstance->Add_RenderGroup((RENDERGROUP)m_iRenderGroup, this);
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

_float CParticleEffect::Ready_Death()
{
	m_pVIBufferCom->Set_Loop(false);
	return m_fMaxLifeTime;
}

void CParticleEffect::Set_Loop(_bool isLoop)
{
	m_pVIBufferCom->Set_Loop(isLoop);
}


HRESULT CParticleEffect::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_ParticleEffect"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	_wstring TextureTag = TEXT("Prototype_Component_Texture_");
	if (m_bTextureUsage[TU_DIFFUSE] == true) {
		/* For.Com_Texture */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TextureTag + m_TextureTag[TU_DIFFUSE],
			TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom[TU_DIFFUSE]))))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK1] == true) {
		/* For.Com_TextureMask1 */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TextureTag + m_TextureTag[TU_MASK1],
			TEXT("Com_TextureMask1"), reinterpret_cast<CComponent**>(&m_pTextureCom[TU_MASK1]))))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK2] == true) {
		/* For.Com_TextureMask2 */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TextureTag + m_TextureTag[TU_MASK2],
			TEXT("Com_TextureMask2"), reinterpret_cast<CComponent**>(&m_pTextureCom[TU_MASK2]))))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK3] == true) {
		/* For.Com_TextureMask3 */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TextureTag + m_TextureTag[TU_MASK3],
			TEXT("Com_TextureMask3"), reinterpret_cast<CComponent**>(&m_pTextureCom[TU_MASK3]))))
			return E_FAIL;
	}

	_wstring strPrototypeTag = TEXT("Prototype_Component_VIBuffer_");
	strPrototypeTag += m_strBufferTag;
	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), strPrototypeTag,
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CParticleEffect::Bind_ShaderResources()
{
	if (m_pSocketMatrix != nullptr)
	{
		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
			return E_FAIL;
	}
	else
	{
		if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
			return E_FAIL;
	}
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;

	if (FAILED(__super::Bind_ShaderResources()))
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
	j["MaxLifeTime"] = m_fMaxLifeTime;
	j["PType"] = m_ePType;
	j["Loop"] = m_isLoop;
	j["Local"] = true;
	

	return j;
}

void CParticleEffect::Deserialize(const json& j)
{
	__super::Deserialize(j);

	if (j.contains("NumInstance"))
		m_iNumInstance = j["NumInstance"].get<_uint>();

	if (j.contains("MaxLifeTime"))
		m_fMaxLifeTime = j["MaxLifeTime"].get<_float>();

	if (j.contains("PType"))
		m_ePType = static_cast<PARTICLETYPE>(j["PType"].get<int>());

	if (j.contains("Loop"))
		m_isLoop = j["Loop"].get<_bool>();

	if (j.contains("Local"))
		m_bLocal = j["Local"].get<_bool>();

	if (j.contains("Name"))
	{
		m_strBufferTag = StringToWString(j["Name"].get<std::string>());
	}
	m_bBillboard = false;

}
