#include "UI_Popup.h"
#include "Dynamic_Text_UI.h"
#include "UI_Manager.h"

CUI_Popup::CUI_Popup(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUI_Container{pDevice, pContext}
{
}

CUI_Popup::CUI_Popup(const CUI_Popup& Prototype)
	:CUI_Container{Prototype}
{
}

json CUI_Popup::Serialize()
{
	json j = __super::Serialize();

	return j;
}

void CUI_Popup::Deserialize(const json& j)
{
	__super::Deserialize(j);
}

HRESULT CUI_Popup::Initialize_Prototype()
{
	m_bCloned = false;

	return S_OK;
}

HRESULT CUI_Popup::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_isActive = false;
	
	m_bCloned = true;

	CUI_Manager::Get_Instance()->Emplace_UI(this, TEXT("Popup"));

	return S_OK;
}

void CUI_Popup::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CUI_Popup::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CUI_Popup::Late_Update(_float fTimeDelta)
{
	if(m_isActive)
		__super::Late_Update(fTimeDelta);
}

HRESULT CUI_Popup::Render()
{
	return S_OK;
}

void CUI_Popup::Set_String(_int iTriggerType)
{
	_wstring strCaption;

	switch (iTriggerType)
	{
	case 0:
		strCaption = L"조사한다.";
		break;
	case 1:
		strCaption = L"말을 건다.";
		break;
	case 2:
		strCaption = L"문을 연다.";
		break;
	default:
		strCaption = L"????";
		break;
	}
	
	if (m_PartObjects.empty())
		return;
	static_cast<CDynamic_Text_UI*>(m_PartObjects.back())->Set_Caption(strCaption);
}

CUI_Popup* CUI_Popup::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Popup* pInstance = new CUI_Popup(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Popup");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Popup::Clone(void* pArg)
{
	CUI_Popup* pInstance = new CUI_Popup(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_Popup");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Popup::Free()
{
	__super::Free();

	
}
