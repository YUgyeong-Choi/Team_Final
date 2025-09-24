#include "Panel_Player_RU.h"
#include "GameInstance.h"
#include "Observer_Player_Status.h"
#include "UI_Text.h"
#include "Dynamic_Text_UI.h"
#include "Egro_Bar.h"

CPanel_Player_RU::CPanel_Player_RU(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUI_Container{pDevice, pContext}
{
}

CPanel_Player_RU::CPanel_Player_RU(const CPanel_Player_RU& Prototype)
	:CUI_Container{Prototype}
{
}

HRESULT CPanel_Player_RU::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPanel_Player_RU::Initialize(void* pArg)
{
	UI_CONTAINER_DESC eDesc = {};
	eDesc.strFilePath = TEXT("../Bin/Save/UI/Panel_Player_RU.json");

	if (FAILED(__super::Initialize(&eDesc)))
		return E_FAIL;

	// 콜백 등록
	// 에르고만 가져오면 될듯?

	if (nullptr == m_pGameInstance->Find_Observer(TEXT("Player_Status")))
	{

		m_pGameInstance->Add_Observer(TEXT("Player_Status"), new CObserver_Player_Status);

	}

	m_pGameInstance->Register_PushCallback(TEXT("Player_Status"), this, [this](_wstring eventType, void* data) {
		if (L"CurrentErgo" == eventType)
		{

			// 증가된 값을 넣어준다.
			m_fErgo = *static_cast<_float*>(data);

			m_isChange = true;
			
			m_PartObjects[0]->Set_Color({ 1.f,1.f,1.f,1.f });

			_int iDelta = static_cast<_int>((m_fErgo - m_fPreErgo));
			_wstring strAdd = {};

			if(iDelta > 0)
				strAdd = L"+" + to_wstring(static_cast<_int>((iDelta)));
			else
				strAdd = to_wstring(static_cast<_int>((iDelta)));

			static_cast<CUI_Text*>(m_PartObjects[0])->Set_Caption(strAdd);
			static_cast<CDynamic_Text_UI*>(m_PartObjects[1])->Set_Caption(to_wstring(static_cast<_int>(m_fErgo)));

			_float fRatio = m_fErgo / m_fMaxErgo;

			if (fRatio > 1.f)
			{
				fRatio = 1.f;
				m_PartObjects[1]->Set_Color({ 0.08f,0.5f,0.5f,1.f });
				m_PartObjects[2]->Set_Color({ 0.08f,0.5f,0.5f,1.f });
			}
			else
			{
				m_PartObjects[1]->Set_Color({ 1.f,1.f,1.f,1.f });
				m_PartObjects[2]->Set_Color({ 1.f,1.f,1.f,1.f });
			}
		
			static_cast<CErgo_Bar*>(m_PartObjects[2])->Set_Ratio(fRatio);


		}
		else if (L"LevelUp" == eventType)
		{

			m_fMaxErgo = *static_cast<_float*>(data);

			_float fRatio = m_fErgo / m_fMaxErgo;

			if (fRatio > 1.f)
			{
				fRatio = 1.f;
				m_PartObjects[1]->Set_Color({ 0.08f,0.5f,0.5f,1.f });
				m_PartObjects[2]->Set_Color({ 0.08f,0.5f,0.5f,1.f });
			}
			else
			{
				m_PartObjects[1]->Set_Color({ 1.f,1.f,1.f,1.f });
				m_PartObjects[2]->Set_Color({ 1.f,1.f,1.f,1.f });
			}

			static_cast<CErgo_Bar*>(m_PartObjects[2])->Set_Ratio(fRatio);

		}
		else if (L"UseErgo" == eventType)
		{

		}
		

		});



	return S_OK;
}

void CPanel_Player_RU::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);

	if (m_bDead)
	{
		m_pGameInstance->Remove_Callback(TEXT("Player_Status"), this);
	}

	if (m_isChange)
	{
		m_fRenderTime -= fTimeDelta;

		if (m_fRenderTime < 0.f)
		{
			m_fRenderTime = 2.f;
			m_isChange = false;
		}
		m_PartObjects[0]->Set_Color({ 1.f,1.f,1.f,1.f });
	}
	else
	{
		m_fPreErgo = m_fErgo;
		m_PartObjects[0]->Set_Color({ 0.f,0.f,0.f,0.f });
	}
}

void CPanel_Player_RU::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CPanel_Player_RU::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CPanel_Player_RU::Render()
{
	return S_OK;
}

CPanel_Player_RU* CPanel_Player_RU::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPanel_Player_RU* pInstance = new CPanel_Player_RU(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPanel_Player_RU");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPanel_Player_RU::Clone(void* pArg)
{
	CPanel_Player_RU* pInstance = new CPanel_Player_RU(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPanel_Player_RU");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPanel_Player_RU::Free()
{
	__super::Free();

}

