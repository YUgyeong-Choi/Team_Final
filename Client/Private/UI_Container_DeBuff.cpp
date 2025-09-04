#include "UI_Container_DeBuff.h"
#include "GameInstance.h"
#include "Observer_Player_Status.h"
#include "Dynamic_UI.h"
#include "Dynamic_Text_UI.h"
#include "DeBuff_Bar.h"

CUI_Container_DeBuff::CUI_Container_DeBuff(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CUI_Container{pDevice, pContext}
{
}

CUI_Container_DeBuff::CUI_Container_DeBuff(const CUI_Container_DeBuff& Prototype)
    :CUI_Container{Prototype}
{
}

HRESULT CUI_Container_DeBuff::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_Container_DeBuff::Initialize(void* pArg)
{
    UI_CONTAINER_DESC eDesc = {};
    eDesc.strFilePath = TEXT("../Bin/Save/UI/DeBuff/DeBuff.json");

    if (FAILED(__super::Initialize(&eDesc)))
        return E_FAIL;

	// 콜백을 등록
	if (nullptr == m_pGameInstance->Find_Observer(TEXT("Player_Status")))
	{

		m_pGameInstance->Add_Observer(TEXT("Player_Status"), new CObserver_Player_Status);

	}

	m_pGameInstance->Register_PushCallback(TEXT("Player_Status"), [this](_wstring eventType, void* data) {
		if (L"Fire" == eventType)
		{
			m_fRatio = *static_cast<float*>(data);

			static_cast<CDynamic_UI*>(m_PartObjects[0])->Set_iTextureIndex(0);
			static_cast<CDeBuff_Bar*>(m_PartObjects[1])->Set_Ratio(m_fRatio);
			static_cast<CDynamic_Text_UI*>(m_PartObjects[2])->Set_Caption(L"과열");
		}


	

		});

    return S_OK;
}

void CUI_Container_DeBuff::Priority_Update(_float fTimeDelta)
{
	if (m_fRatio > 0.01f)
		__super::Priority_Update(fTimeDelta);
}

void CUI_Container_DeBuff::Update(_float fTimeDelta)
{
	if (m_fRatio > 0.01f)
		__super::Update(fTimeDelta);
}

void CUI_Container_DeBuff::Late_Update(_float fTimeDelta)
{
	if (m_fRatio > 0.01f)
		__super::Late_Update(fTimeDelta);
}

HRESULT CUI_Container_DeBuff::Render()
{
    return S_OK;
}

void CUI_Container_DeBuff::Update_Info()
{
}

CUI_Container_DeBuff* CUI_Container_DeBuff::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Container_DeBuff* pInstance = new CUI_Container_DeBuff(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Container_DeBuff");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Container_DeBuff::Clone(void* pArg)
{
	CUI_Container_DeBuff* pInstance = new CUI_Container_DeBuff(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_Container_DeBuff");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Container_DeBuff::Free()
{
	__super::Free();

	for (auto& pObj : m_PartObjects)
		Safe_Release(pObj);

	m_PartObjects.clear();
}