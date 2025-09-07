#include "UI_Popup.h"
#include "Dynamic_Text_UI.h"
#include "UI_Manager.h"
#include "GameInstance.h"

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



	m_vOriginPos = static_cast<CDynamic_UI*>(m_PartObjects[0])->Get_Pos();

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

void CUI_Popup::Active_Update(_bool isActive)
{

	__super::Active_Update(isActive);

	
	m_isChange = isActive;

	if (false == isActive && true == m_bDoOnce)
		m_bDoOnce = false;
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
	case 3:
		strCaption = L"별바라기를 복구한다.";
		break;
	case 4:
		strCaption = L"별바라기를 사용한다.";
		break;
	default:
		strCaption = L"????";
		break;
	}
	
	if (m_PartObjects.empty())
		return;

	if (m_isChange && !m_bDoOnce)
	{
		_float2 vOriginSize = static_cast<CDynamic_UI*>(m_PartObjects[0])->Get_Size();

		_float2 vChangeSize = m_pGameInstance->Calc_Draw_Range(TEXT("Font_Medium"), strCaption.c_str());

		static_cast<CDynamic_UI*>(m_PartObjects[0])->Get_TransfomCom()->Scaling(vChangeSize.x * 1.2f, vOriginSize.y, 0.f);
		static_cast<CDynamic_UI*>(m_PartObjects[1])->Get_TransfomCom()->Scaling(vChangeSize.x * 1.2f, vOriginSize.y, 0.f);

		

		static_cast<CDynamic_UI*>(m_PartObjects[0])->Set_Position(m_vOriginPos.x + vChangeSize.x * 0.15f, m_vOriginPos.y - g_iWinSizeY * 0.01f);
		static_cast<CDynamic_UI*>(m_PartObjects[1])->Set_Position(m_vOriginPos.x + vChangeSize.x * 0.15f, m_vOriginPos.y - g_iWinSizeY * 0.0575f);

		m_bDoOnce = true;
	}

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
