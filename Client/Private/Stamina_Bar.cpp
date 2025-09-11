#include "Stamina_Bar.h"
#include "GameInstance.h"
#include "Observer_Player_Status.h"
#include "UI_Feature_Position.h"

CStamina_Bar::CStamina_Bar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CDynamic_UI{pDevice, pContext}
{
}

CStamina_Bar::CStamina_Bar(const CStamina_Bar& Prototype)
    :CDynamic_UI (Prototype)
{
}



HRESULT CStamina_Bar::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CStamina_Bar::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strProtoTag = TEXT("Prototype_GameObject_UI_Stamina_Bar");

	// 콜백을 등록
	if (nullptr == m_pGameInstance->Find_Observer(TEXT("Player_Status")))
	{

		m_pGameInstance->Add_Observer(TEXT("Player_Status"), new CObserver_Player_Status);

	}

	m_pGameInstance->Register_PushCallback(TEXT("Player_Status"), [this](_wstring eventType, void* data) {
		if (L"CurrentStamina" == eventType)
		{
			m_fCurrentStamina = *static_cast<_float*>(data);


		}
		else if (L"MaxStamina" == eventType)
		{
			_float fMaxStamina = m_fMaxStamina;
			_float fPreScaleRatio = m_fScaleRatio;
			m_fMaxStamina = *static_cast<_float*>(data);

			if (m_fMaxStamina != 0.f && fMaxStamina != 0.f)
				m_fScaleRatio = m_fMaxStamina / fMaxStamina;


			// 사이즈가 커지면
			if (m_fScaleRatio > 1.f)
			{
				_vector fSize = m_pTransformCom->Get_Scale();

				_float fOriginSizeX = fSize.m128_f32[0];

				fSize.m128_f32[0] *= m_fScaleRatio / fPreScaleRatio;

				m_pTransformCom->SetUp_Scale(fSize.m128_f32[0], fSize.m128_f32[1], fSize.m128_f32[2]);

				_float fDelta = fSize.m128_f32[0] - fOriginSizeX;

				for (auto& pFeature : m_pUIFeatures)
				{
					if (pFeature->Get_Tag().find(L"Pos") != _wstring::npos)
					{
						_float2 fPos = static_cast<CUI_Feature_Position*>(pFeature)->Get_Position(false);
						fPos.x += fDelta * 0.43f / g_iWinSizeX;
						static_cast<CUI_Feature_Position*>(pFeature)->Set_Position(false, fPos);

						break;
					}
				}
			}
		}

		m_fRatio = (m_fCurrentStamina) / m_fMaxStamina;

		});


	Ready_Component(m_strTextureTag);

	if (nullptr == pArg)
		return S_OK;



	return S_OK;
}

void CStamina_Bar::Priority_Update(_float fTimeDelta)
{
}

void CStamina_Bar::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CStamina_Bar::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CStamina_Bar::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(D_UI_HPBAR)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;


	return S_OK;
}

HRESULT CStamina_Bar::Bind_ShaderResources()
{
	__super::Bind_ShaderResources();

	// 넘겨줘

	if (FAILED(m_pBackTextureCom->Bind_ShaderResource(m_pShaderCom, "g_BackgroundTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pGradationCom->Bind_ShaderResource(m_pShaderCom, "g_GradationTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_BarRatio", &m_fRatio, sizeof(_float))))
		return E_FAIL;

	_float isHpbar = 0.f;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_IsPlayer", &isHpbar, sizeof(_float))))
		return E_FAIL;



	return S_OK;
}

HRESULT CStamina_Bar::Ready_Component(const wstring& strTextureTag)
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
	if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Bar_Gradation"),
		TEXT("Com_Texture_Gradation"), reinterpret_cast<CComponent**>(&m_pGradationCom))))
		return E_FAIL;

	return S_OK;
}

CStamina_Bar* CStamina_Bar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CStamina_Bar* pInstance = new CStamina_Bar(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CStamina_Bar");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CStamina_Bar::Clone(void* pArg)
{
	CStamina_Bar* pInstance = new CStamina_Bar(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CStamina_Bar");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CStamina_Bar::Free()
{
	__super::Free();

	Safe_Release(m_pBackTextureCom);
	Safe_Release(m_pGradationCom);
}