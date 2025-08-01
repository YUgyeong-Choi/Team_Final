#include "Static_UI.h"
#include "GameInstance.h"


json CStatic_UI::Serialize()
{
	json j;
	j = __super::Serialize();

	j["Texturetag"] = WStringToString(m_strTextureTag);
	j["iTextureLevel"] = m_iTextureLevel;
	j["iTextureIndex"] = m_iTextureIndex;
	j["iPassIndex"] = m_iPassIndex;

	return j;
}

void CStatic_UI::Deserialize(const json& j)
{
	__super::Deserialize(j);

	string textureTag = j["Texturetag"].get<string>();
	m_strTextureTag = StringToWStringU8(textureTag);

	m_iTextureLevel = j["iTextureLevel"];
	m_iTextureIndex = j["iTextureIndex"];
	m_iPassIndex = j["iPassIndex"];

	Ready_Components_File(m_strTextureTag);

	Update_Data();
}

CStatic_UI::CStatic_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject{ pDevice, pContext }
{
}

CStatic_UI::CStatic_UI(const CStatic_UI& Prototype)
	: CUIObject( Prototype )
{
}

HRESULT CStatic_UI::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CStatic_UI::Initialize(void* pArg)
{
	

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	m_strProtoTag = TEXT("Prototype_GameObject_Static_UI");

	if (nullptr == pArg)
		return S_OK;

	STATIC_UI_DESC* pDesc = static_cast<STATIC_UI_DESC*>(pArg);

	m_strTextureTag = pDesc->strTextureTag;
	m_iTextureLevel = pDesc->iTextureLevel;

	if (FAILED(Ready_Components(m_strTextureTag)))
		return E_FAIL;

	m_iPassIndex = pDesc->iPassIndex;
	m_iTextureIndex = pDesc->iTextureIndex;
	
	

	return S_OK;
}

void CStatic_UI::Priority_Update(_float fTimeDelta)
{
}

void CStatic_UI::Update(_float fTimeDelta)
{
}

void CStatic_UI::Late_Update(_float fTimeDelta)
{
	if (!m_isActive)
		return;


	if(!m_isDeferred)
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_UI, this);
	else
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_UI_DEFERRED, this);
}

HRESULT CStatic_UI::Render()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_iTextureIndex)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_Color", &m_vColor, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(m_iPassIndex)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void CStatic_UI::Update_UI_From_Tool(STATIC_UI_DESC& eDesc)
{
	m_vColor = eDesc.vColor;
	m_fX = eDesc.fX;
	m_fY = eDesc.fY;
	m_fOffset = eDesc.fOffset;
	m_fSizeX = eDesc.fSizeX;
	m_fSizeY = eDesc.fSizeY;
	m_iPassIndex = eDesc.iPassIndex;
	m_iTextureIndex = eDesc.iTextureIndex;
	m_fRotation = eDesc.fRotation;

	D3D11_VIEWPORT			ViewportDesc{};
	_uint					iNumViewports = { 1 };

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);


	m_pTransformCom->Scaling(m_fSizeX, m_fSizeY);

	m_pTransformCom->Rotation(0.f, 0.f, XMConvertToRadians(m_fRotation));

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(m_fX - ViewportDesc.Width * 0.5f, -m_fY + ViewportDesc.Height * 0.5f, m_fOffset, 1.f));
}

void CStatic_UI::Update_UI_From_Tool(void* pArg)
{
	STATIC_UI_DESC* pDesc = static_cast<STATIC_UI_DESC*>(pArg);

	m_vColor = pDesc->vColor;
	m_fX = pDesc->fX;
	m_fY = pDesc->fY;
	m_fOffset = pDesc->fOffset;
	m_fSizeX = pDesc->fSizeX;
	m_fSizeY = pDesc->fSizeY;
	m_iPassIndex = pDesc->iPassIndex;
	m_iTextureIndex = pDesc->iTextureIndex;
	m_fRotation = pDesc->fRotation;

	D3D11_VIEWPORT			ViewportDesc{};
	_uint					iNumViewports = { 1 };

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);


	m_pTransformCom->Scaling(m_fSizeX, m_fSizeY);

	m_pTransformCom->Rotation(0.f, 0.f, XMConvertToRadians(m_fRotation));

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(m_fX - ViewportDesc.Width * 0.5f, -m_fY + ViewportDesc.Height * 0.5f, m_fOffset, 1.f));

}

HRESULT CStatic_UI::Ready_Components(const wstring& strTextureTag)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_UI"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;
	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;
	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(m_iTextureLevel, strTextureTag,
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CStatic_UI::Ready_Components_File(const wstring& strTextureTag)
{
	__super::Ready_Components_File(strTextureTag);

	m_strTextureTag = strTextureTag;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_UI"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;
	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;
	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(m_iTextureLevel, strTextureTag,
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	return S_OK;
}

CStatic_UI* CStatic_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CStatic_UI* pInstance = new CStatic_UI(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CStatic_UI");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CStatic_UI::Clone(void* pArg)
{
	CStatic_UI* pInstance = new CStatic_UI(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CStatic_UI");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CStatic_UI::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);

}
