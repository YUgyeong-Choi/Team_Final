#include "Layer.h"
#include "ContainerObject.h"

CLayer::CLayer()
{
}

CComponent* CLayer::Get_Component(const _wstring& strComponentTag, _uint iIndex)
{
	auto	iter = m_GameObjects.begin();

	for (size_t i = 0; i < iIndex; i++)
		++iter;

	
	return (*iter)->Get_Component(strComponentTag);
}

CComponent* CLayer::Get_Component(_uint iPartID, const _wstring& strComponentTag, _uint iIndex)
{
	auto	iter = m_GameObjects.begin();

	for (size_t i = 0; i < iIndex; i++)
		++iter;

	return static_cast<CContainerObject*>(*iter)->Get_Component(iPartID, strComponentTag);
}

HRESULT CLayer::Add_GameObject(CGameObject* pGameObject)
{
	if (nullptr == pGameObject)
		return E_FAIL;

	m_GameObjects.push_back(pGameObject);

	return S_OK;
}

void CLayer::Priority_Update(_float fTimeDelta)
{
	for (auto iter = m_GameObjects.begin(); iter != m_GameObjects.end(); )
	{
		CGameObject* pGameObject = *iter;

		if (pGameObject != nullptr)
		{
			if (!pGameObject->Get_isActive())
			{
				++iter;
				continue;
			}

			_float fScaledDelta = pGameObject->Get_TimeScale()* fTimeDelta;
			pGameObject->Priority_Update(fScaledDelta);

			if (pGameObject->Get_bDead())
			{
				Safe_Release(pGameObject);  // 리소스 해제
				iter = m_GameObjects.erase(iter);  // 리스트에서 제거
				continue;  // erase 후 iter가 이미 다음 요소를 가리킴
			}
		}

		++iter;
	}
		
}

void CLayer::Update(_float fTimeDelta)
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (nullptr != pGameObject)
		{
			if (!pGameObject->Get_isActive())
				continue;

			_float fScaledDelta = pGameObject->Get_TimeScale() * fTimeDelta;
			pGameObject->Update(fScaledDelta);
		}
	}
}

void CLayer::Late_Update(_float fTimeDelta)
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (nullptr != pGameObject)
		{
			//if (!pGameObject->Get_isActive())
			//	continue;

			_float fScaledDelta = pGameObject->Get_TimeScale() * fTimeDelta;
			pGameObject->Late_Update(fScaledDelta);
		}
	}
}

void CLayer::Last_Update(_float fTimeDelta)
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (nullptr != pGameObject)
		{
			if (!pGameObject->Get_isActive())
				continue;

			_float fScaledDelta = pGameObject->Get_TimeScale() * fTimeDelta;
			pGameObject->Last_Update(fScaledDelta);
		}
	}
}

CGameObject* CLayer::Get_Object(_uint iIndex)
{
	if (m_GameObjects.size() <= iIndex)
		return nullptr;

	auto	iter = m_GameObjects.begin();

	for (size_t i = 0; i < iIndex; ++i)
		++iter;

	return *iter;
}


CGameObject* CLayer::Get_LastObject()
{
	return m_GameObjects.back();
}

list<class CGameObject*>& CLayer::Get_ObjectList()
{
	return m_GameObjects;
}

CLayer* CLayer::Create()
{
	return new CLayer();
}

void CLayer::Free()
{
	__super::Free();

	for (auto& pGameObject : m_GameObjects)
		Safe_Release(pGameObject);
	m_GameObjects.clear();
}
