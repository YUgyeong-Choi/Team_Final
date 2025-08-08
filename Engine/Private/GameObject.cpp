#include "GameObject.h"
#include "GameInstance.h"

CGameObject::CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pDevice);
}

CGameObject::CGameObject(const CGameObject& Prototype)
	: m_pDevice { Prototype.m_pDevice }
	, m_pContext { Prototype.m_pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pDevice);

	//클론임
	m_bCloned = true;
}

CComponent* CGameObject::Get_Component(const _wstring& strComponentTag)
{
	auto	iter = m_Components.find(strComponentTag);
	if (iter == m_Components.end())
		return nullptr;

	return iter->second;
}

HRESULT CGameObject::Initialize_Prototype()
{	

	return S_OK;
}

HRESULT CGameObject::Initialize(void* pArg)
{
	m_pTransformCom = CTransform::Create(m_pDevice, m_pContext);
	if (nullptr == m_pTransformCom)
		return E_FAIL;	

	if (FAILED(m_pTransformCom->Initialize(pArg)))
		return E_FAIL;

	m_Components.emplace(g_strTransformTag, m_pTransformCom);

	Safe_AddRef(m_pTransformCom);

	if (nullptr != pArg)
	{
		GAMEOBJECT_DESC* pDesc = static_cast<GAMEOBJECT_DESC*>(pArg);
		lstrcpy(m_szName, pDesc->szName);
	}

	return S_OK;
}

void CGameObject::Priority_Update(_float fTimeDelta)
{
}

void CGameObject::Update(_float fTimeDelta)
{

}

void CGameObject::Late_Update(_float fTimeDelta)
{

}

void CGameObject::Last_Update(_float fTimeDelta)
{
}

HRESULT CGameObject::Render()
{

	return S_OK;
}

void CGameObject::Compute_ViewZ(_vector pCamPos , const _vector* pTargetPos)
{
	_vector vDiff = pCamPos - *pTargetPos;
	m_fViewZ = XMVectorGetX(XMVector3Length(vDiff));
}


_bool CGameObject::Get_bDead()
{
	return m_bDead;
}

void CGameObject::PrintMatrix(const char* szName, const _matrix& mat)
{
	_float4x4 matFloat;
	XMStoreFloat4x4(&matFloat, mat);

	printf("[%s Matrix]\n", szName);
	printf(" %.2f %.2f %.2f %.2f\n", matFloat._11, matFloat._12, matFloat._13, matFloat._14);
	printf(" %.2f %.2f %.2f %.2f\n", matFloat._21, matFloat._22, matFloat._23, matFloat._24);
	printf(" %.2f %.2f %.2f %.2f\n", matFloat._31, matFloat._32, matFloat._33, matFloat._34);
	printf(" %.2f %.2f %.2f %.2f\n\n", matFloat._41, matFloat._42, matFloat._43, matFloat._44);
}

HRESULT CGameObject::Add_Component(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg)
{
	CComponent*	pComponent = static_cast<CComponent*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_COMPONENT, iPrototypeLevelIndex, strPrototypeTag, pArg));
	if (nullptr == pComponent)
		return E_FAIL;

	m_Components.emplace(make_pair(strComponentTag, pComponent));

	*ppOut = pComponent;

	Safe_AddRef(pComponent);

	return S_OK;
}


HRESULT CGameObject::Replace_Component(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg)
{
	// 둘다 nullptr이 아니면 실제 있는거니까
	if (ppOut && *ppOut)
		Safe_Release(*ppOut);

	CComponent* pComponent = static_cast<CComponent*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_COMPONENT, iPrototypeLevelIndex, strPrototypeTag, pArg));
	if (nullptr == pComponent)
		return E_FAIL;

	Remove_Component(strComponentTag);

	m_Components.emplace(make_pair(strComponentTag, pComponent));


	if (ppOut)
	{
		*ppOut = pComponent;
		Safe_AddRef(pComponent);   
	}

	return S_OK;
}


void CGameObject::Remove_Component(const _wstring& strComponentTag)
{
	auto	iter = m_Components.find(strComponentTag);
	if (iter != m_Components.end())
	{
		Safe_Release(iter->second);
		m_Components.erase(iter);
	}
}

void CGameObject::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pTransformCom);

	for (auto& Pair : m_Components)
		Safe_Release(Pair.second);
	m_Components.clear();

	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
}
