#include "Dynamic_UI.h"
#include "GameInstance.h"

CDynamic_UI::CDynamic_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUIObject{pDevice, pContext}
{
}

CDynamic_UI::CDynamic_UI(const CDynamic_UI& Prototype)
	:CUIObject{Prototype}
{
}

HRESULT CDynamic_UI::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDynamic_UI::Initialize(void* pArg)
{
	DYNAMIC_UI_DESC* pDesc = static_cast<DYNAMIC_UI_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strTextureTag = pDesc->strTextureTag;

	if (FAILED(Ready_Components(m_strTextureTag)))
		return E_FAIL;

	m_iPassIndex = pDesc->iPassIndex;
	m_iTextureIndex = pDesc->iTextureIndex;

	m_iStartFrame = pDesc->iStartFrame;
	m_iEndFrame = pDesc->iEndFrame;

	m_fOffsetUV = pDesc->fOffsetUV;

	m_iWidth = static_cast<_int>(1 / m_fOffsetUV.x);
	m_iHeight = static_cast<_int>(1 / m_fOffsetUV.y);

	m_iUIType = pDesc->iUIType;


	return S_OK;
}

void CDynamic_UI::Priority_Update(_float fTimeDelta)
{
}

void CDynamic_UI::Update(_float fTimeDelta)
{
	// 나중에 객체 만들어서 빼기
	if (m_iUIType == 2)
	{
		m_fDuration += fTimeDelta;

		if (m_fDuration > 0.15f)
		{
			m_iCount;

			++m_iCount;

			if (m_iCount >= m_iWidth * m_iHeight)
				m_iCount = 0;

			int iRow = m_iCount / m_iWidth;
			int iCol = m_iCount % m_iWidth;

			m_fUV.x = iCol * m_fOffsetUV.x;
			m_fUV.y = iRow * m_fOffsetUV.y;

			m_fDuration = 0.f;

		}
			
	}
}

void CDynamic_UI::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_UI, this);
}

HRESULT CDynamic_UI::Render()
{

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(m_iPassIndex)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void CDynamic_UI::Update_UI_From_Tool(DYNAMIC_UI_DESC& eDesc)
{
	// sequence type에 따라 일단 바꿔보자

	
}


HRESULT CDynamic_UI::Ready_Components(const wstring& strTextureTag)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_DynamicUI"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;
	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;
	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), strTextureTag,
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CDynamic_UI::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_iTextureIndex)))
		return E_FAIL;

	// 나중에 객체 만들어서 빼기
	if (m_iUIType == 2)
	{
		if(FAILED(m_pShaderCom->Bind_RawValue("g_fTexcoord", &m_fUV, sizeof(_float2))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_fTileSize", &m_fOffsetUV, sizeof(_float2))))
			return E_FAIL;
	}


	return S_OK;
}

CDynamic_UI* CDynamic_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDynamic_UI* pInstance = new CDynamic_UI(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CDynamic_UI");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDynamic_UI::Clone(void* pArg)
{
	CDynamic_UI* pInstance = new CDynamic_UI(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CDynamic_UI");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDynamic_UI::Free()
{
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	__super::Free();
}

