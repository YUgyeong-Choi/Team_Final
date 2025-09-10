#include "HP_Bar.h"
#include "GameInstance.h"
#include "Observer_Player_Status.h"
#include "UI_Feature_Position.h"

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
			m_fCurrentHP = *static_cast<_float*>(data);


		}
		else if (L"MaxHP" == eventType)
		{
			_float fMaxHp = m_fMaxHp;
			_float fPreScaleRatio = m_fScaleRatio;
			m_fMaxHp = *static_cast<_float*>(data);

			if (fMaxHp != 0.f)
				m_fScaleRatio = m_fMaxHp / fMaxHp;
			

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

				//_float2 fPos = Get_Pos();

				//fPos.x += fDelta * 0.5f;

				//Set_Position(fPos.x, fPos.y);
				
			}
		}
			
		m_fRatio = (m_fCurrentHP) / m_fMaxHp;

		if (m_fRatio > m_fCurrentRatio)
			m_isPlus = true;
		else
			m_isPlus = false;
		
		});

	
	Ready_Component(m_strTextureTag);

	if (nullptr == pArg)
		return S_OK;



	return S_OK;
}

void CHP_Bar::Priority_Update(_float fTimeDelta)
{
	
}

void CHP_Bar::Update(_float fTimeDelta)
{
	// 일단 하고 나중에 고쳐
	__super::Update(fTimeDelta);

	if (m_isPlus)
	{
		m_fCurrentRatio = clamp(m_fCurrentRatio + fTimeDelta * 1.5f, 0.f, m_fRatio);
	}
	else
	{
		m_fCurrentRatio = clamp(m_fCurrentRatio - fTimeDelta * 1.5f, m_fRatio, 1.f);
	}
	
}

void CHP_Bar::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CHP_Bar::Render()
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

HRESULT CHP_Bar::Bind_ShaderResources()
{
	__super::Bind_ShaderResources();

	// 넘겨줘

	if (FAILED(m_pBackTextureCom->Bind_ShaderResource(m_pShaderCom, "g_BackgroundTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pGradationCom->Bind_ShaderResource(m_pShaderCom, "g_GradationTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_BarRatio", &m_fCurrentRatio, sizeof(_float))))
		return E_FAIL;

	_float isHpbar = 1.f;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_IsHpBar", &isHpbar, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

HRESULT CHP_Bar::Ready_Component(const wstring& strTextureTag)
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

	Safe_Release(m_pBackTextureCom);
	Safe_Release(m_pGradationCom);
}
