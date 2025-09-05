#include "MeshEffect.h"

#include "GameInstance.h"

CMeshEffect::CMeshEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffectBase{ pDevice, pContext }
{

}

CMeshEffect::CMeshEffect(const CMeshEffect& Prototype)
	: CEffectBase(Prototype)
{

}

HRESULT CMeshEffect::Initialize_Prototype()
{
	m_KeyFrames.push_back(EFFKEYFRAME{});
	return S_OK;
}

HRESULT CMeshEffect::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	DESC* pDesc = static_cast<DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_eEffectType = EFF_PARTICLE;
	return S_OK;
}

void CMeshEffect::Priority_Update(_float fTimeDelta)
{
	if (m_isActive == false || m_isEffectActive == false)
		return;

	m_fTimeAcc += fTimeDelta;

}

void CMeshEffect::Update(_float fTimeDelta)
{

	__super::Update(fTimeDelta);

	return;
}

void CMeshEffect::Late_Update(_float fTimeDelta)
{
	//m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_BLEND, this);

	//m_iRenderGroup = (_int)RENDERGROUP::RG_EFFECT_GLOW;
	if (m_isActive == false || m_isEffectActive == false)
		return;

	m_pGameInstance->Add_RenderGroup((RENDERGROUP)m_iRenderGroup, this);
}

HRESULT CMeshEffect::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pShaderCom->Begin(m_iShaderPass);

		m_pModelCom->Render(i);
	}

	return S_OK;
}

_float CMeshEffect::Ready_Death()
{
	m_isActive = false;
	return 0.f;
}

HRESULT CMeshEffect::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_MeshEffect"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	_wstring strPrototypeTag = TEXT("Prototype_Component_Model_");
	strPrototypeTag += m_strModelTag;
	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), strPrototypeTag,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
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
	return S_OK;
}

HRESULT CMeshEffect::Bind_ShaderResources()
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

	if (FAILED(__super::Bind_ShaderResources()))
		return E_FAIL;

	return S_OK;
}

CMeshEffect* CMeshEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeshEffect* pInstance = new CMeshEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeshEffect::Clone(void* pArg)
{
	CMeshEffect* pInstance = new CMeshEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMeshEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeshEffect::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
}

json CMeshEffect::Serialize()
{
	json j = __super::Serialize();

	j["ModelTag"] = WStringToString(m_strModelTag);

	return j;
}

void CMeshEffect::Deserialize(const json& j)
{
	__super::Deserialize(j);
	if (j.contains("ModelTag"))
		m_strModelTag = StringToWString(j["ModelTag"].get<string>());
	m_bBillboard = false;
}
