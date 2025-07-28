#include "StaticMesh_Instance.h"
#include "GameInstance.h"

CStaticMesh_Instance::CStaticMesh_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CStaticMesh{ pDevice, pContext }
{

}

CStaticMesh_Instance::CStaticMesh_Instance(const CStaticMesh_Instance& Prototype)
	: CStaticMesh(Prototype)
{

}

HRESULT CStaticMesh_Instance::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CStaticMesh_Instance::Initialize(void* pArg)
{
	//인스턴싱으로 이니셜라이즈

	STATICMESHINSTANCE_DESC* InstanceDesc = static_cast<STATICMESHINSTANCE_DESC*>(pArg);

	m_iNumInstance = InstanceDesc->iNumInstance;

	m_eLevelID = InstanceDesc->m_eLevelID;

	m_szMeshID = InstanceDesc->szMeshID;

	m_iRender = InstanceDesc->iRender;

	InstanceDesc->fSpeedPerSec = 0.f;
	InstanceDesc->fRotationPerSec = 0.f;

	if (FAILED(CGameObject::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(InstanceDesc->WorldMatrix);

	//m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(m_InitPos.x, m_InitPos.y, m_InitPos.z, 1.f));
	//m_pTransformCom->SetUp_Scale(InstanceDesc->m_vInitScale.x, InstanceDesc->m_vInitScale.y, InstanceDesc->m_vInitScale.z);

	return S_OK;
}

void CStaticMesh_Instance::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CStaticMesh_Instance::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CStaticMesh_Instance::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CStaticMesh_Instance::Render()
{
	//인스턴싱으로 렌더

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0)))
			return E_FAIL;

		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CStaticMesh_Instance::Ready_Components(void* pArg)
{
  	CStaticMesh::STATICMESH_DESC* StaicMeshDESC = static_cast<STATICMESH_DESC*>(pArg);

	/* Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxMesh_Instance")),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(m_eLevelID), StaicMeshDESC->szModelPrototypeTag/*_wstring(TEXT("Prototype_Component_Model_")) + m_szMeshID*/,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CStaticMesh_Instance::Bind_ShaderResources()
{
	return __super::Bind_ShaderResources();
}

CStaticMesh_Instance* CStaticMesh_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CStaticMesh_Instance* pGameInstance = new CStaticMesh_Instance(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CStaticMesh_Instance");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CStaticMesh_Instance::Clone(void* pArg)
{
	CStaticMesh_Instance* pGameInstance = new CStaticMesh_Instance(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CStaticMesh_Instance");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CStaticMesh_Instance::Free()
{
	__super::Free();
}
