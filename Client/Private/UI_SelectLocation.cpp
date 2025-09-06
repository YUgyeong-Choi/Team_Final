#include "UI_SelectLocation.h"
#include "GameInstance.h"

CUI_SelectLocation::CUI_SelectLocation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUI_Container{pDevice, pContext}
{
}

CUI_SelectLocation::CUI_SelectLocation(const CUI_SelectLocation& Prototype)
	:CUI_Container{Prototype}
{
}

HRESULT CUI_SelectLocation::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_SelectLocation::Initialize(void* pArg)
{
	UI_SELECT_LOCATION_DESC* pDesc = static_cast<UI_SELECT_LOCATION_DESC*>(pArg);

	m_pTarget = pDesc->pTarget;

	UI_CONTAINER_DESC eDesc = {};

	// 
	eDesc.strFilePath = TEXT("../Bin/Save/UI/SelectLocation/SelectLocation_Background.json");

	if (FAILED(__super::Initialize(&eDesc)))
		return E_FAIL;

	eDesc.strFilePath = TEXT("../Bin/Save/UI/SelectLocation/SelectLocation_LocationImg.json");

	m_pLocationImg = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

	eDesc.strFilePath = TEXT("../Bin/Save/UI/SelectLocation/SelectLocation_Button.json");

	m_pBehindButtons = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));


	Ready_Button();


	return S_OK;
}

void CUI_SelectLocation::Priority_Update(_float fTimeDelta)
{
}

void CUI_SelectLocation::Update(_float fTimeDelta)
{
}

void CUI_SelectLocation::Late_Update(_float fTimeDelta)
{
}

HRESULT CUI_SelectLocation::Render()
{
	return E_NOTIMPL;
}

HRESULT CUI_SelectLocation::Ready_Button()
{
	// 별바라기 레이어를 찾고
	// 몇개가 활성화 되있는지 찾고
	// 활성화 된거만 tag 받아와서
	// 그 개수 만큼 만든다

	return S_OK;
}

CUI_SelectLocation* CUI_SelectLocation::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CGameObject* CUI_SelectLocation::Clone(void* pArg)
{
	return nullptr;
}

void CUI_SelectLocation::Free()
{
}
