#include "UI_Script_Text.h"
#include "Dynamic_Text_UI.h"
#include "GameInstance.h"

CUI_Script_Text::CUI_Script_Text(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUI_Container{pDevice, pContext}
{
}

CUI_Script_Text::CUI_Script_Text(const CUI_Script_Text& Prototype)
	:CUI_Container{Prototype}
{
}




HRESULT CUI_Script_Text::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Script_Text::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CUI_Script_Text::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CUI_Script_Text::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CUI_Script_Text::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CUI_Script_Text::Render()
{
	return S_OK;
}

void CUI_Script_Text::Update_Script(const string strText)
{
	static_cast<CDynamic_Text_UI*>(m_PartObjects[0])->Set_Caption(StringToWStringU8(strText));


	_float2 vSize = m_pGameInstance->Calc_Draw_Range(L"Font_Medium", StringToWStringU8(strText).c_str());

	vSize.x *= 0.75f * (g_iWinSizeX / 1600.f);
	vSize.y *= 0.75f * (g_iWinSizeY / 900.f);

	m_PartObjects[1]->Get_TransfomCom()->Scaling(vSize.x, vSize.y);


}


CUI_Script_Text* CUI_Script_Text::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Script_Text* pInstance = new CUI_Script_Text(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Script_Text");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Script_Text::Clone(void* pArg)
{
	CUI_Script_Text* pInstance = new CUI_Script_Text(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_Script_Text");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Script_Text::Free()
{
	__super::Free();

}
