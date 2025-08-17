#include "Icon_Weapon.h"
#include "GameInstance.h"

CIcon_Weapon::CIcon_Weapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CDynamic_UI{pDevice, pContext}
{
}

CIcon_Weapon::CIcon_Weapon(const CIcon_Weapon& Prototype)
	:CDynamic_UI{Prototype}
{
}

HRESULT CIcon_Weapon::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CIcon_Weapon::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strProtoTag = TEXT("Prototype_GameObject_UI_Icon_Weapon");


	if (nullptr == pArg)
		return S_OK;


	return S_OK;
}

void CIcon_Weapon::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CIcon_Weapon::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CIcon_Weapon::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CIcon_Weapon::Render()
{
	__super::Render();

	return S_OK;
}

void CIcon_Weapon::Update_ICon(_wstring& strTextureTag)
{

	if (strTextureTag.empty())
	{

		m_strTextureTag = TEXT("Prototype_Component_Texture_Weapon_Default");

		if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Weapon_Default"),
			TEXT("Com_Texture_Item"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
			return;
	}
	else
	{
		m_strTextureTag = strTextureTag;

		if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), strTextureTag,
			TEXT("Com_Texture_Item"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
			return;
	}

	
	
	
}



CIcon_Weapon* CIcon_Weapon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CIcon_Weapon* pInstance = new CIcon_Weapon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CIcon_Weapon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CIcon_Weapon::Clone(void* pArg)
{
	CIcon_Weapon* pInstance = new CIcon_Weapon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CIcon_Weapon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CIcon_Weapon::Free()
{
	__super::Free();


}