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

	// ���õ� �������� ���� �� �ڵ� ����
	if (m_pSelectItem == nullptr && pItem != nullptr)
	{
		m_pSelectItem = pItem;
		m_iSelectIndex = iIndex;

	}
	m_ViewItems.push_back(pItem);
}

void CBelt::Use_SelectItem(_bool isActive)
{
	if (nullptr == m_pSelectItem)
		return;

	m_pSelectItem->Activate(isActive);

}

void CBelt::DeselectItem()
{
	m_pSelectItem = nullptr;
}

void CBelt::Change_Next_Item()
{
	size_t iSize = m_Items.size();

	int iCount = {};
	for (auto& pItem : m_Items)
	{
		if (nullptr != pItem)
			++iCount;

	}

	// �ƹ��͵� ������ ���� ��.
	if (0 >= iCount)
	{

		return;
	}
		
	

	// index �ٲٴ°� ����
	int iStartIndex = (m_iSelectIndex + 1) % iSize;

	for (int i = 0; i < iSize; ++i)
	{
		int iCheckIndex = (iStartIndex + i) % iSize;

		if (m_Items[iCheckIndex] != nullptr)
		{
			// ���� ����
			m_pSelectItem = m_Items[iCheckIndex];
			m_iSelectIndex = iCheckIndex;

			// ViewItems ����
			m_ViewItems.clear();  
			for (int j = 0; j < iSize; ++j)
			{
				int iViewIndex = (m_iSelectIndex + j) % iSize;
				if (m_Items[iViewIndex] != nullptr)
					m_ViewItems.push_back(m_Items[iViewIndex]);
			}

			return;
		}
	}
}

_bool CBelt::Find_Item(const _wstring& strProtoTag)
{

	for (int i =0; i< m_Items.size(); ++i)
	{
		if (nullptr != m_Items[i])
		{
			if (m_Items[i]->Get_ProtoTag() == strProtoTag)
			{
				m_iSelectIndex = i - 1;
				Change_Next_Item();
				return true;
			}
		}
		
	}

	return false;
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

