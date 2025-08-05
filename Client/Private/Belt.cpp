#include "Belt.h"
#include "GameInstance.h"

CBelt::CBelt(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject{pDevice, pContext}
{
}

CBelt::CBelt(const CBelt& Prototype)
	:CGameObject{Prototype}
{
}

HRESULT CBelt::Initialize_Prototype()
{
	return S_OK;
}


HRESULT CBelt::Initialize(void* pArg)
{
	if(FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_Items.resize(3);



	return S_OK;
}

void CBelt::Priority_Update(_float fTimeDelta)
{
	for (auto& item : m_Items)
	{
		if (nullptr != item)
			item->Priority_Update(fTimeDelta);
	}
}

void CBelt::Update(_float fTimeDelta)
{
	for (auto& item : m_Items)
	{
		if (nullptr != item)
			item->Update(fTimeDelta);
	}
}

void CBelt::Late_Update(_float fTimeDelta)
{
	for (auto& item : m_Items)
	{
		if (nullptr != item)
			item->Late_Update(fTimeDelta);
	}
}

HRESULT CBelt::Render()
{
	return E_NOTIMPL;
}

void CBelt::Add_Item(CItem* pItem, _int iIndex)
{
	if (iIndex >= m_Items.size())
		return;

	m_Items[iIndex] = pItem;
}

void CBelt::Use_SelectItem()
{
	if (nullptr == m_pSelectItem)
		return;

	m_pSelectItem->Activate();

}

void CBelt::DeselectItem()
{
	m_pSelectItem = nullptr;
}

void CBelt::Change_Next_Item()
{
	size_t iCount = m_Items.size();
	int nextIndex = (m_iSelectIndex + 1) % iCount;

	while (nextIndex != m_iSelectIndex)
	{
		if (m_Items[nextIndex] != nullptr) // 아이템 있으면 반환
		{
			// 순서 바꿔서 저장
			rotate(m_Items.begin(), m_Items.begin() + nextIndex, m_Items.end());

			m_pSelectItem = m_Items[0];
			m_iSelectIndex = 0;
			return;
		}
			
		nextIndex = (nextIndex + 1) % iCount;
	}

	// 다 돌고, 처음 인덱스 다시 체크
	if (m_Items[m_iSelectIndex] != nullptr)
	{
		rotate(m_Items.begin(), m_Items.begin() + m_iSelectIndex, m_Items.end());

		m_pSelectItem = m_Items[0];
		m_iSelectIndex = 0;
		return;
	}
	// 다 돌아도 없으면 벨트에 아이템이 없으니까 
	m_pSelectItem = nullptr;
}

CBelt* CBelt::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBelt* pInstance = new CBelt(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CBelt");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CBelt::Clone(void* pArg) 
{
	CBelt* pInstance = new CBelt(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CBelt");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CBelt::Free()
{
	__super::Free();

	for (auto& pItem : m_Items)
		Safe_Release(pItem);

}

