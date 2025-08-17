#include "Panel_Player_RD.h"
#include "GameInstance.h"
#include "Observer_Weapon.h"
#include "Icon_Weapon.h"
#include "Mana_Bar.h"
#include "Weapon.h"

CPanel_Player_RD::CPanel_Player_RD(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUI_Container{pDevice, pContext}
{
}

CPanel_Player_RD::CPanel_Player_RD(const CPanel_Player_RD& Prototype)
	:CUI_Container{Prototype}
{
}

void CPanel_Player_RD::Set_isReverse(_bool isReverse)
{
	__super::Set_isReverse(isReverse);

	m_pSkillType->Set_isReverse(isReverse);
	m_pManaCost->Set_isReverse(isReverse);
	m_pKeyIcon->Set_isReverse(isReverse);
	m_pWeaponTexture->Set_isReverse(isReverse);
}

HRESULT CPanel_Player_RD::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPanel_Player_RD::Initialize(void* pArg)
{

	UI_CONTAINER_DESC eDesc = {};
	eDesc.strFilePath = TEXT("../Bin/Save/UI/Weapon/BackGround.json");

	if (FAILED(__super::Initialize(&eDesc)))
		return E_FAIL;

	// 파일 읽어와서 각각 때려 박아 
	eDesc.strFilePath = TEXT("../Bin/Save/UI/Weapon/Default.json");
	m_pWeaponTexture = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Weapon/SkillType.json");
	m_pSkillType = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Weapon/ManaCost.json");
	m_pManaCost = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Weapon/KeyIcon.json");
	m_pKeyIcon = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));



	static_cast<CMana_Bar*>(m_pManaCost->Get_PartUI()[0])->Set_isUseWeapon();
	static_cast<CMana_Bar*>(m_pManaCost->Get_PartUI()[1])->Set_isUseWeapon();

	// 콜백을 등록하자
	if (nullptr == m_pGameInstance->Find_Observer(TEXT("Weapon_Status")))
	{

		m_pGameInstance->Add_Observer(TEXT("Weapon_Status"), new CObserver_Weapon);

	}

	m_pGameInstance->Register_PushCallback(TEXT("Weapon_Status"), [this](_wstring eventType, void* data) {

		if (L"EquipWeapon" == eventType)
		{
			if (nullptr == data)
			{
				m_strWeaponTextureTag = TEXT("Prototype_Component_Texture_Weapon_Default");
				static_cast<CIcon_Weapon*>(m_pWeaponTexture->Get_PartUI()[0])->Update_ICon(m_strWeaponTextureTag);
				static_cast<CIcon_Weapon*>(m_pWeaponTexture->Get_PartUI()[1])->Update_ICon(m_strWeaponTextureTag);
				return;
			}


			auto pWeapon = static_cast<CWeapon*>(data);

			_wstring strMeshTag = pWeapon->Get_MeshName();

			if (strMeshTag.empty())
			{
				m_strWeaponTextureTag = TEXT("Prototype_Component_Texture_Weapon_Default");
				static_cast<CIcon_Weapon*>(m_pWeaponTexture->Get_PartUI()[0])->Update_ICon(m_strWeaponTextureTag);
				static_cast<CIcon_Weapon*>(m_pWeaponTexture->Get_PartUI()[1])->Update_ICon(m_strWeaponTextureTag);
				return;
			}
			

			if (L"PlayerWeapon" == strMeshTag)
			{
				m_strWeaponTextureTag = TEXT("Prototype_Component_Texture_Weapon_Bayonet");
				static_cast<CIcon_Weapon*>(m_pWeaponTexture->Get_PartUI()[0])->Update_ICon(m_strWeaponTextureTag);
				static_cast<CIcon_Weapon*>(m_pWeaponTexture->Get_PartUI()[1])->Update_ICon(m_strWeaponTextureTag);
			}


			SKILL_DESC weaponDesc[2] = { pWeapon->Get_SkillDesc(0), pWeapon->Get_SkillDesc(1) };

			static_cast<CDynamic_UI*>(m_pSkillType->Get_PartUI()[0])->Set_iTextureIndex(weaponDesc[0].iSkillType);
			static_cast<CDynamic_UI*>(m_pSkillType->Get_PartUI()[1])->Set_iTextureIndex(weaponDesc[1].iSkillType);

			static_cast<CMana_Bar*>(m_pManaCost->Get_PartUI()[0])->Set_MaxMana(weaponDesc[0].fManaCost);
			static_cast<CMana_Bar*>(m_pManaCost->Get_PartUI()[1])->Set_MaxMana(weaponDesc[1].fManaCost);

		}


		});



	return S_OK;
}

void CPanel_Player_RD::Priority_Update(_float fTimeDelta)
{

	for (size_t i = 0; i < m_PartObjects.size() - 1; ++i)
	{
		m_PartObjects[i]->Priority_Update(fTimeDelta);
	}
	for (auto& pWeaponTexture : m_pWeaponTexture->Get_PartUI())
	{
		pWeaponTexture->Priority_Update(fTimeDelta);
	}


	if (!m_strWeaponTextureTag.empty() && L"Prototype_Component_Texture_Weapon_Default" != m_strWeaponTextureTag)
	{
		m_PartObjects.back()->Priority_Update(fTimeDelta);

		for (auto& pTexture : m_pSkillType->Get_PartUI())
		{
			pTexture->Priority_Update(fTimeDelta);
		}

		for (auto& pTexture : m_pManaCost->Get_PartUI())
		{
			pTexture->Priority_Update(fTimeDelta);
		}

		for (auto& pTexture : m_pKeyIcon->Get_PartUI())
		{
			pTexture->Priority_Update(fTimeDelta);
		}
	}
}

void CPanel_Player_RD::Update(_float fTimeDelta)
{
	for (size_t i = 0; i < m_PartObjects.size() - 1; ++i)
	{
		m_PartObjects[i]->Update(fTimeDelta);
	}
	for (auto& pWeaponTexture : m_pWeaponTexture->Get_PartUI())
	{
		pWeaponTexture->Update(fTimeDelta);
	}


	if (!m_strWeaponTextureTag.empty() && L"Prototype_Component_Texture_Weapon_Default" != m_strWeaponTextureTag)
	{
		m_PartObjects.back()->Update(fTimeDelta);

		for (auto& pTexture : m_pSkillType->Get_PartUI())
		{
			pTexture->Update(fTimeDelta);
		}

		for (auto& pTexture : m_pManaCost->Get_PartUI())
		{
			pTexture->Update(fTimeDelta);
		}

		for (auto& pTexture : m_pKeyIcon->Get_PartUI())
		{
			pTexture->Update(fTimeDelta);
		}
	}
}

void CPanel_Player_RD::Late_Update(_float fTimeDelta)
{
	for (size_t i = 0; i < m_PartObjects.size() - 1; ++i)
	{
		m_PartObjects[i]->Late_Update(fTimeDelta);
	}
	for (auto& pWeaponTexture : m_pWeaponTexture->Get_PartUI())
	{
		pWeaponTexture->Late_Update(fTimeDelta);
	}


	if (!m_strWeaponTextureTag.empty() && L"Prototype_Component_Texture_Weapon_Default" != m_strWeaponTextureTag)
	{
		m_PartObjects.back()->Late_Update(fTimeDelta);

		for (auto& pTexture : m_pSkillType->Get_PartUI())
		{
			pTexture->Late_Update(fTimeDelta);
		}

		for (auto& pTexture : m_pManaCost->Get_PartUI())
		{
			pTexture->Late_Update(fTimeDelta);
		}

		for (auto& pTexture : m_pKeyIcon->Get_PartUI())
		{
			pTexture->Late_Update(fTimeDelta);
		}
	}
}

HRESULT CPanel_Player_RD::Render()
{
	return S_OK;
}



CPanel_Player_RD* CPanel_Player_RD::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{

	CPanel_Player_RD* pInstance = new CPanel_Player_RD(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPanel_Player_RD");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPanel_Player_RD::Clone(void* pArg)
{
	CPanel_Player_RD* pInstance = new CPanel_Player_RD(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPanel_Player_RD");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPanel_Player_RD::Free()
{
	__super::Free();

	Safe_Release(m_pWeaponTexture);
	Safe_Release(m_pSkillType);
	Safe_Release(m_pManaCost);
	Safe_Release(m_pKeyIcon);
}
