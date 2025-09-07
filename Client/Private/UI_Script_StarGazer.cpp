#include "UI_Script_StarGazer.h"
#include "GameInstance.h"
#include "UI_Button.h"
#include "Dynamic_Text_UI.h"

CUI_Script_StarGazer::CUI_Script_StarGazer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUI_Container{pDevice, pContext}
{
}

CUI_Script_StarGazer::CUI_Script_StarGazer(const CUI_Script_StarGazer& Prototype)
	:CUI_Container{Prototype}
{
}

HRESULT CUI_Script_StarGazer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Script_StarGazer::Initialize(void* pArg)
{

	UI_CONTAINER_DESC eDesc{};

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Script/Sciprt_Stargazer_Background.json");

	// 백그라운드
	if (FAILED(__super::Initialize(&eDesc)))
		return E_FAIL;

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Script/Sciprt_Stargazer_Button.json");
	m_pButtons = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));
	// Text

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Script/Script_Talk_Text.json");
	m_pText = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

	static_cast<CDynamic_Text_UI*>(m_pText->Get_PartUI()[0])->Set_Caption(L"별바라기");
	static_cast<CDynamic_Text_UI*>(m_pText->Get_PartUI()[1])->Set_Caption(L"별바라기를 사용한다.");

	Active_Update(true);

	return S_OK;

}

void CUI_Script_StarGazer::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);

	m_pButtons->Priority_Update(fTimeDelta);
	m_pText->Priority_Update(fTimeDelta);
}

void CUI_Script_StarGazer::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	m_pButtons->Update(fTimeDelta);
	m_pText->Update(fTimeDelta);

	for (auto& pButton : m_pButtons->Get_PartUI())
	{
		if (pButton->Get_isActive())
			static_cast<CUI_Button*>(pButton)->Check_MouseHover();
	}


}

void CUI_Script_StarGazer::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	m_pButtons->Late_Update(fTimeDelta);
	m_pText->Late_Update(fTimeDelta);
}

HRESULT CUI_Script_StarGazer::Render()
{
	return S_OK;
}

void CUI_Script_StarGazer::Update_Script(const string strName, const string strText)
{
	static_cast<CDynamic_Text_UI*>(m_pText->Get_PartUI()[0])->Set_Caption(StringToWStringU8(strName));
	static_cast<CDynamic_Text_UI*>(m_pText->Get_PartUI()[1])->Set_Caption(StringToWStringU8(strText));
}

void CUI_Script_StarGazer::Active_Update(_bool isActive)
{
	__super::Active_Update(isActive);


	m_pButtons->Active_Update(isActive);
	m_pText->Active_Update(isActive);
}

_int CUI_Script_StarGazer::Check_Click_Button()
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

CUI_Script_StarGazer* CUI_Script_StarGazer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Script_StarGazer* pInstance = new CUI_Script_StarGazer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Script_StarGazer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Script_StarGazer::Clone(void* pArg)
{
	CUI_Script_StarGazer* pInstance = new CUI_Script_StarGazer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_Script_StarGazer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Script_StarGazer::Free()
{
	__super::Free();

	Safe_Release(m_pButtons);
	Safe_Release(m_pText);
}