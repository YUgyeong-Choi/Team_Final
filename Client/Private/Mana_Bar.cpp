#include "Mana_Bar.h"
#include "GameInstance.h"
#include "Observer_Player_Status.h"

CMana_Bar::CMana_Bar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CDynamic_UI{pDevice, pContext}
{
}

CMana_Bar::CMana_Bar(const CMana_Bar& Prototype)
	:CDynamic_UI{Prototype}
{
}


HRESULT CMana_Bar::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMana_Bar::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strProtoTag = TEXT("Prototype_GameObject_UI_Mana_Bar");

	// 콜백을 등록
	if (nullptr == m_pGameInstance->Find_Observer(TEXT("Player_Status")))
	{

		m_pGameInstance->Add_Observer(TEXT("Player_Status"), new CObserver_Player_Status);

	}

	m_pGameInstance->Register_PushCallback(TEXT("Player_Status"), [this](_wstring eventType, void* data) {
		if (L"CurrentMana" == eventType)
		{
			m_iCurrentMana = *static_cast<int*>(data);


		}
		else if (L"MaxMana" == eventType)
		{

			if(!m_isUseWeapon)
				m_iMaxMana = *static_cast<int*>(data);


		}

		

		});


	Ready_Component(m_strTextureTag);

	if (nullptr == pArg)
		return S_OK;



	return S_OK;
}

void CMana_Bar::Priority_Update(_float fTimeDelta)
{
}

void CMana_Bar::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMana_Bar::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMana_Bar::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(D_UI_MANABAR)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;


	return S_OK;
}

HRESULT CMana_Bar::Bind_ShaderResources()
{
	__super::Bind_ShaderResources();

	// 넘겨줘

	if (FAILED(m_pBackTextureCom->Bind_ShaderResource(m_pShaderCom, "g_BackgroundTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pGradationCom->Bind_ShaderResource(m_pShaderCom, "g_GradationTexture", 0)))
		return E_FAIL;

	// //
	// 그릴 칸 수. 칸 사이 마진, 몇 칸 꽉 차있는지, 현재 칸에 얼마 비율로 차 있는지.
	_float4 vManaDesc = {};
	
	if (!m_isUseWeapon)
	{
		vManaDesc = { float(m_iMaxMana) / 100, 0.01f, float(m_iCurrentMana) / 100, float(m_iCurrentMana % 100) / 100.f };
	}
	else
	{
		if(float(m_iCurrentMana % 100) / 100.f < 1.f)
			vManaDesc = { float(m_iMaxMana) / 100, 0.01f, float(m_iCurrentMana) / 100, 0.f };
		else
			vManaDesc = { float(m_iMaxMana) / 100, 0.01f, float(m_iCurrentMana) / 100, 1.f };
	}


	if (FAILED(m_pShaderCom->Bind_RawValue("g_ManaDesc", &vManaDesc, sizeof(_float4))))
		return E_FAIL;

	return S_OK;
}

HRESULT CMana_Bar::Ready_Component(const wstring& strTextureTag)
{
	if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_DynamicUI"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;
	/* For.Com_VIBuffer */
	if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Bar_Border"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;


	/* For.Com_Texture */
	if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Bar_Background"),
		TEXT("Com_Texture_Back"), reinterpret_cast<CComponent**>(&m_pBackTextureCom))))
		return E_FAIL;


	/* For.Com_Texture */
	if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Bar_Gradation_Side"),
		TEXT("Com_Texture_Gradation"), reinterpret_cast<CComponent**>(&m_pGradationCom))))
		return E_FAIL;

	return S_OK;
}



CMana_Bar* CMana_Bar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMana_Bar* pInstance = new CMana_Bar(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMana_Bar");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMana_Bar::Clone(void* pArg)
{
	CMana_Bar* pInstance = new CMana_Bar(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMana_Bar");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMana_Bar::Free()
{
	__super::Free();

	Safe_Release(m_pBackTextureCom);
	Safe_Release(m_pGradationCom);
}

