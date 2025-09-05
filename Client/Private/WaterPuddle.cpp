#include "WaterPuddle.h"
#include "GameInstance.h"



CWaterPuddle::CWaterPuddle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CWaterPuddle::CWaterPuddle(const CWaterPuddle& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CWaterPuddle::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CWaterPuddle::Initialize(void* pArg)
{
	CWaterPuddle::WATER_DESC* WaterDESC = static_cast<WATER_DESC*>(pArg);

	if (FAILED(__super::Initialize(WaterDESC)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	m_szMeshID = WaterDESC->szMeshID;
	m_pTransformCom->Set_WorldMatrix(WaterDESC->WorldMatrix);

	return S_OK;
}

void CWaterPuddle::Priority_Update(_float fTimeDelta)
{

}

void CWaterPuddle::Update(_float fTimeDelta)
{
}

void CWaterPuddle::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_WATERPUDDLE, this);
}

HRESULT CWaterPuddle::Render()
{
#ifdef _DEBUG
	if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXActorCom))
	{
		if (m_pGameInstance->Get_RenderMapCollider())
		{
			if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom)))
				return E_FAIL;
		}
	}
#endif

	return S_OK;
}

HRESULT CWaterPuddle::Render_WaterPuddle()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();
	for (_uint i = 0; i < iNumMesh; i++)
	{

		m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);

		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CWaterPuddle::Ready_Components(void* pArg)
{
	CWaterPuddle::WATER_DESC* WaterDESC = static_cast<WATER_DESC*>(pArg);

	/* Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxPBRMesh")),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), _wstring(TEXT("Prototype_Component_Model_WaterPuddleA")),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
		TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CWaterPuddle::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

CWaterPuddle* CWaterPuddle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWaterPuddle* pGameInstance = new CWaterPuddle(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CWaterPuddle");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CWaterPuddle::Clone(void* pArg)
{
	CWaterPuddle* pGameInstance = new CWaterPuddle(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CWaterPuddle");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CWaterPuddle::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pPhysXActorCom);
}
