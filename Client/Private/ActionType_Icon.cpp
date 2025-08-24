#include "ActionType_Icon.h"
#include "GameInstance.h"
#include "UI_Feature.h"

CActionType_Icon::CActionType_Icon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CDynamic_UI{pDevice, pContext}
{
}

CActionType_Icon::CActionType_Icon(const CActionType_Icon& Prototype)
	:CDynamic_UI{Prototype}
{
}

HRESULT CActionType_Icon::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CActionType_Icon::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;


	// 기본 적인 텍스처 이미지 저장해놓기

	m_strProtoTag = TEXT("Prototype_GameObject_UI_Icon_ActionType");


	//Prototype_Component_Texture_ActionType_Effect

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_ActionType_Effect"),
		TEXT("Com_Texture_Select"), reinterpret_cast<CComponent**>(&m_pEffectTextureCom))))
		return E_FAIL;
	
	if (nullptr == pArg)
		return S_OK;


	return S_OK;

}

void CActionType_Icon::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CActionType_Icon::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	

}

void CActionType_Icon::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CActionType_Icon::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(D_UI_ACTION_ICON)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	switch (m_iTextureIndex)
	{
	case 0:
		m_strCaption = TEXT("귀뚜라미의 길 : 균형");
		break;
	case 1:
		m_strCaption = TEXT("서자들의 길 : 민첩");
		break;
	case 2:
		m_strCaption = TEXT("청소부들의 길 : 강인");
		break;


	default:
		break;
	}


	if (!m_strCaption.empty())
	{
		_vector vColor = { 1.f, 1.f, 1.f, 1.f };

		if (!m_isSelect)
		{
			vColor = { 0.7f,0.7f,0.7f,0.7f };
			m_pGameInstance->Draw_Font_Centered(TEXT("Font_Medium"), m_strCaption.c_str(), { m_fX ,m_fY + m_fSizeY * 0.4f }, vColor, 0.f, { 0.f,0.f }, 0.65f);
		}
		else
		{
			m_pGameInstance->Draw_Font_Centered(TEXT("Font_Bold"), m_strCaption.c_str(), { m_fX ,m_fY + m_fSizeY * 0.4f }, vColor, 0.f, { 0.f,0.f }, 0.65f);
		}
			

		

		
	}
		


	return S_OK;
}

HRESULT CActionType_Icon::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (m_isSelect)
	{
		if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_iTextureIndex)))
			return E_FAIL;
	}
	else
	{
		if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_iTextureIndex + 3)))
			return E_FAIL;
	}

	
	if (FAILED(m_pEffectTextureCom->Bind_ShaderResource(m_pShaderCom, "g_HighlightTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_Color", &m_vColor, sizeof(_float4))))
		return E_FAIL;

	// 기본값을 던지고, 추가로 덮어쓰자

	if (FAILED(m_pShaderCom->Bind_RawValue("g_Alpha", &m_fCurrentAlpha, sizeof(_float))))
		return E_FAIL;


	for (auto& pFeature : m_pUIFeatures)
	{
		if (nullptr != pFeature)
			pFeature->Bind_ShaderResources(m_pShaderCom);
	}

	_float4  vIconDesc = { _float(m_isSelect) , 0.f,0.f,0.f };

	if (FAILED(m_pShaderCom->Bind_RawValue("g_ItemDesc", &vIconDesc, sizeof(_float4))))
		return E_FAIL;
	

	return S_OK;
}

_bool CActionType_Icon::Check_Hover()
{
	return _bool();
}


CActionType_Icon* CActionType_Icon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CActionType_Icon* pInstance = new CActionType_Icon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CActionType_Icon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CActionType_Icon::Clone(void* pArg)
{
	CActionType_Icon* pInstance = new CActionType_Icon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CActionType_Icon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CActionType_Icon::Free()
{
	__super::Free();

	Safe_Release(m_pEffectTextureCom);
	
}
