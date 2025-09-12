#include "UI_Pickup_Item.h"
#include "GameInstance.h"
#include "UI_Manager.h"

CUI_Pickup_Item::CUI_Pickup_Item(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUI_Container{pDevice, pContext}
{
}

CUI_Pickup_Item::CUI_Pickup_Item(const CUI_Pickup_Item& Prototype)
	:CUI_Container{Prototype}
{
}

HRESULT CUI_Pickup_Item::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_Pickup_Item::Initialize(void* pArg)
{
    UI_CONTAINER_DESC eDesc{};
	eDesc.useLifeTime = false;
	eDesc.fLifeTime = 8.f;

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Popup/Item_Pickup_BG.json");

	// 백그라운드
	if (FAILED(__super::Initialize(&eDesc)))
		return E_FAIL;

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Popup/Item_Pickup_Text.json");

	m_pDescription = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));


	CUI_Manager::Get_Instance()->Emplace_UI(this, TEXT("Pickup_Item"));

	Active_Update(false);

	


    return S_OK;
}

void CUI_Pickup_Item::Priority_Update(_float fTimeDelta)
{
	if (m_fLifeTime <= 0.f)
	{
		Active_Update(false);
		return;
	}

	if(m_fLifeTime > 0.f)
	m_fLifeTime -= fTimeDelta;

    __super::Priority_Update(fTimeDelta);

	if (nullptr != m_pDescription)
		m_pDescription->Priority_Update(fTimeDelta);
	
}

void CUI_Pickup_Item::Update(_float fTimeDelta)
{
	if (m_fLifeTime <= 0)
	{
		
		return;
	}


    __super::Update(fTimeDelta);

	if (nullptr != m_pDescription)
		m_pDescription->Update(fTimeDelta);
}

void CUI_Pickup_Item::Late_Update(_float fTimeDelta)
{
	if (m_fLifeTime <= 0)
	{
		return;
	}
    __super::Late_Update(fTimeDelta);

	if (nullptr != m_pDescription)
		m_pDescription->Late_Update(fTimeDelta);
}

HRESULT CUI_Pickup_Item::Render()
{
    return S_OK;
}

void CUI_Pickup_Item::Active_Update(_bool isActive)
{
	__super::Active_Update(isActive);

	if (nullptr != m_pDescription)
	{
		m_pDescription->Active_Update(isActive);
	}
	

	if (isActive)
	{
		CUI_Manager::Get_Instance()->Sound_Play("SE_UI_RewardItem_02");
	}
}

void CUI_Pickup_Item::Update_Description(_int itemType)
{
	if (itemType == 1 || ENUM_CLASS(ITEM_TAG::END) == itemType)
		return;

	// 다시 넣어준다
	Safe_Release(m_pDescription);

	UI_CONTAINER_DESC eDesc{};
	eDesc.useLifeTime = false;
	eDesc.fLifeTime = 8.f;

	_wstring strFilePath = L"../Bin/Save/UI/Popup/Item_Pickup_Text_" + to_wstring(itemType) + L".json";


	eDesc.strFilePath = strFilePath;

	m_pDescription = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

	m_fLifeTime = 8.f;

	for (auto& pPart : m_PartObjects)
		pPart->Reset();

	for (auto& pPart : m_pDescription->Get_PartUI())
		pPart->Reset();
	//

}

CUI_Pickup_Item* CUI_Pickup_Item::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Pickup_Item* pInstance = new CUI_Pickup_Item(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Pickup_Item");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Pickup_Item::Clone(void* pArg)
{
	CUI_Pickup_Item* pInstance = new CUI_Pickup_Item(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_Pickup_Item");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Pickup_Item::Free()
{
	__super::Free();

	Safe_Release(m_pDescription);
}
