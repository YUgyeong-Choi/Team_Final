#include "TriggerItemLamp.h"

#include "GameInstance.h"

CTriggerItemLamp::CTriggerItemLamp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CTriggerItem{ pDevice, pContext }
{
}

CTriggerItemLamp::CTriggerItemLamp(const CTriggerItemLamp& Prototype)
    : CTriggerItem(Prototype)
{
}

HRESULT CTriggerItemLamp::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CTriggerItemLamp::Initialize(void* pArg)
{
	CTriggerItem::TRIGGERITEM_DESC* pDesc = static_cast<TRIGGERITEM_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg))) {
		return E_FAIL;
	}

	if (FAILED(Ready_Components())) {
		return E_FAIL;
	}

	return S_OK;
}

void CTriggerItemLamp::Priority_Update(_float fTimeDelta)
{

}

void CTriggerItemLamp::Update(_float fTimeDelta)
{
}

void CTriggerItemLamp::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

HRESULT CTriggerItemLamp::Render()
{
	if (FAILED(Bind_Shader()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	_float fEmissive = 0.f;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &fEmissive, sizeof(_float))))
		return E_FAIL;

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
			return E_FAIL;

		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CTriggerItemLamp::Bind_Shader()
{
	/* [ 월드 스페이스 넘기기 ] */
	_float4x4 matrix = m_pTransformCom->Get_World4x4();
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &matrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

HRESULT CTriggerItemLamp::Ready_Components()
{	
	/* Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxPBRMesh")),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Prototype_Component_Model_PlayerLamp"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	return S_OK;
}


CTriggerItemLamp* CTriggerItemLamp::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTriggerItemLamp* pInstance = new CTriggerItemLamp(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTriggerItemLamp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CTriggerItemLamp::Clone(void* pArg)
{
	CTriggerItemLamp* pInstance = new CTriggerItemLamp(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTriggerItemLamp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTriggerItemLamp::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
