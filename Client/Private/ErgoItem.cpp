#include "ErgoItem.h"
#include "GameInstance.h"
#include "EffectContainer.h"
#include "Effect_Manager.h"

CErgoItem::CErgoItem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CItem{ pDevice, pContext }
{
}

CErgoItem::CErgoItem(const CErgoItem& Prototype)
	:CItem(Prototype)
{
}

HRESULT CErgoItem::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CErgoItem::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_Effect()))
		return E_FAIL;
	
	ERGOITEM_DESC* pDesc = static_cast<ERGOITEM_DESC*>(pArg);

	m_pTransformCom->Set_WorldMatrix(pDesc->WorldMatrix);

	m_eItemTag = pDesc->eItemTag;

	return S_OK;
}

void CErgoItem::Priority_Update(_float fTimeDelta)
{



}

void CErgoItem::Update(_float fTimeDelta)
{

}

void CErgoItem::Late_Update(_float fTimeDelta)
{
	//m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);

}

HRESULT CErgoItem::Render()
{
	/*if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
		{

		}

		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}*/

	return S_OK;
}

HRESULT CErgoItem::Bind_ShaderResources()
{
	//if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CErgoItem::Ready_Components()
{
	/* Com_Shader */
	//if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxPBRMesh")),
	//	TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
	//	return E_FAIL;

	/* For.Com_Model */ 	//아이템 테스트용 모델
	//if (FAILED(__super::Add_Component(m_iLevelID, TEXT("Prototype_Component_Model_ErgoItem"),
	//	TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CErgoItem::Ready_Effect()
{
	CEffectContainer::DESC desc = {};
	desc.pSocketMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixIdentity());
	MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_ErgoItem_M3P1_WB"), &desc);

	return S_OK;
}

CErgoItem* CErgoItem::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CErgoItem* pInstance = new CErgoItem(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CErgoItem");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CErgoItem::Clone(void* pArg)
{
	CErgoItem* pInstance = new CErgoItem(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CErgoItem");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CErgoItem::Free()
{
	__super::Free();

	//Safe_Release(m_pModelCom);
	//Safe_Release(m_pShaderCom);
}
