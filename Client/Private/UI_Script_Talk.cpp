#include "UI_Script_Talk.h"
#include "GameInstance.h"
#include "UI_Button.h"
#include "Dynamic_Text_UI.h"

CUI_Script_Talk::CUI_Script_Talk(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUI_Container{pDevice, pContext}
{
}

CUI_Script_Talk::CUI_Script_Talk(const CUI_Script_Talk& Prototype)
	:CUI_Container{Prototype}
{
}

HRESULT CUI_Script_Talk::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Script_Talk::Initialize(void* pArg)
{
	// 백그라운드
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	// 버튼

	UI_CONTAINER_DESC eDesc{};

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Script/Script_Talk_Button.json");
	m_pButtons = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));
	// Text

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Script/Script_Talk_Text.json");
	m_pText = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

	static_cast<CDynamic_UI*>(m_PartObjects.back())->Set_isUVmove(true);

	return S_OK;
}

void CUI_Script_Talk::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);

	m_pButtons->Priority_Update(fTimeDelta);
	m_pText->Priority_Update(fTimeDelta);
}

void CUI_Script_Talk::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	m_pButtons->Update(fTimeDelta);
	m_pText->Update(fTimeDelta);

	for (auto& pButton : m_pButtons->Get_PartUI())
	{
		if(pButton->Get_isActive())
		static_cast<CUI_Button*>(pButton)->Check_MouseHover();
	}

	

}

void CUI_Script_Talk::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	m_pButtons->Late_Update(fTimeDelta);
	m_pText->Late_Update(fTimeDelta);
}

HRESULT CUI_Script_Talk::Render()
{
	return S_OK;
}

void CUI_Script_Talk::Update_Script(const string strName, const string strText, _bool isAuto)
{
	static_cast<CDynamic_Text_UI*>(m_pText->Get_PartUI()[0])->Set_Caption(StringToWStringU8(strName));
	static_cast<CDynamic_Text_UI*>(m_pText->Get_PartUI()[1])->Set_Caption(StringToWStringU8(strText));

	static_cast<CDynamic_UI*>(m_PartObjects.back())->Set_isActive(isAuto);
}

void CUI_Script_Talk::Active_Update(_bool isActive)
{
	
	__super::Active_Update(isActive);


	m_pButtons->Active_Update(isActive);
	m_pText->Active_Update(isActive);


}

_int CUI_Script_Talk::Check_Click_Button()
{
	if (m_pGameInstance->Mouse_Down(DIM::LBUTTON))
	{
		for (int i = 0; i < m_pButtons->Get_PartUI().size(); ++i)
		{
			if (static_cast<CUI_Button*>(m_pButtons->Get_PartUI()[i])->Check_MousePos())
			{
				return i;
			}
		}
	}

	return -1;
}

CUI_Script_Talk* CUI_Script_Talk::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Script_Talk* pInstance = new CUI_Script_Talk(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Script_Talk");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Script_Talk::Clone(void* pArg)
{
	CUI_Script_Talk* pInstance = new CUI_Script_Talk(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_Script_Talk");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Script_Talk::Free()
{
	__super::Free();

	Safe_Release(m_pButtons);
	Safe_Release(m_pText);
}
