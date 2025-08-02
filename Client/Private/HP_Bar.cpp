#include "HP_Bar.h"
#include "GameInstance.h"
#include "Observer_Player_Status.h"

CHP_Bar::CHP_Bar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CDynamic_UI{pDevice,pContext}
{
}

CHP_Bar::CHP_Bar(const CHP_Bar& Prototype)
	:CDynamic_UI{Prototype}
{
}

HRESULT CHP_Bar::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHP_Bar::Initialize(void* pArg)
{

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strProtoTag = TEXT("Prototype_GameObject_UI_HP_Bar");

	// 콜백을 등록
	if (nullptr == m_pGameInstance->Find_Observer(TEXT("Player_Status")))
	{

		m_pGameInstance->Add_Observer(TEXT("Player_Status"), new CObserver_Player_Status);

	}

	m_pGameInstance->Register_PushCallback(TEXT("Player_Status"), [this](_wstring eventType, void* data) {
		if (L"CurrentHP" == eventType)
		{
			m_iCurrentHP = *static_cast<int*>(data);


		}
		else if (L"MaxHP" == eventType)
		{
			m_iMaxHP = *static_cast<int*>(data);


		}
			
		m_fRatio = float(m_iCurrentHP) / m_iMaxHP;
		
		});

	


	if (nullptr == pArg)
		return S_OK;



	return S_OK;
}

void CHP_Bar::Priority_Update(_float fTimeDelta)
{
	
}

void CHP_Bar::Update(_float fTimeDelta)
{

}

void CHP_Bar::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CHP_Bar::Render()
{

	//	임시용
	_wstring text = L"현재 체력 : " + to_wstring(m_iCurrentHP);
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), {g_iWinSizeX*0.5, g_iWinSizeY * 0.5}, XMVectorSet(1.f, 0.f, 0.f, 1.f));

	return S_OK;
}

CHP_Bar* CHP_Bar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHP_Bar* pInstance = new CHP_Bar(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CHP_Bar");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHP_Bar::Clone(void* pArg)
{
	CHP_Bar* pInstance = new CHP_Bar(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CHP_Bar");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHP_Bar::Free()
{
	__super::Free();

}
