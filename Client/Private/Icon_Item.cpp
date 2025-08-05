#include "Icon_Item.h"
#include "GameInstance.h"
#include "UI_Feature.h"

CIcon_Item::CIcon_Item(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CDynamic_UI{pDevice, pContext}
{
}

CIcon_Item::CIcon_Item(const CIcon_Item& Prototype)
	:CDynamic_UI{Prototype}
{
}

HRESULT CIcon_Item::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CIcon_Item::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strProtoTag = TEXT("Prototype_GameObject_UI_Icon_Item");

	m_strTextureTag = TEXT("Prototype_Component_Texture_Slot_Background");


	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Slot_Select"),
		TEXT("Com_Texture_Select"), reinterpret_cast<CComponent**>(&m_pEffectTextureCom))))
		return E_FAIL;
	
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Slot_Input"),
		TEXT("Com_Texture_Input"), reinterpret_cast<CComponent**>(&m_pInputTextureCom))))
		return E_FAIL;
	// 콜백으로 아이템 바뀌면 아이템 정보 받아오도록 만들어두자


	if (nullptr == pArg)
		return S_OK;


	return S_OK;
}

void CIcon_Item::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CIcon_Item::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (m_isInput)
	{
		m_fInputTime += fTimeDelta;

		if (m_fInputTime >= m_fDuration)
		{
			m_fInputTime = 0.f;
			m_isInput = false;
		}
	}
}

void CIcon_Item::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CIcon_Item::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(D_UI_ICON_ITEM)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	if (!m_strCaption.empty())
		m_pGameInstance->Draw_Font_Centered(TEXT("Font_Medium"), m_strCaption.c_str(), { m_fX + m_fSizeX * 0.2f,m_fY + m_fSizeY * 0.25f }, { 1.f,1.f,1.f,1.f }, 0.f,{0.f,0.f}, 0.8f);


	return S_OK;
}

void CIcon_Item::Update_ICon(ITEM_DESC* pDesc)
{
	if (!pDesc->strPrototag.empty())
	{
		// 아이템 프로토타입 이름 따라서 텍스처 정해줘서 replace하기
		if (pDesc->strPrototag.find(L"Ramp") != pDesc->strPrototag.npos)
		{

			m_strTextureTag = TEXT("Prototype_Component_Texture_Ramp");

			
		}
		else if (pDesc->strPrototag.find(L"Portion") != pDesc->strPrototag.npos)
		{
			m_strTextureTag = TEXT("Prototype_Component_Texture_Portion");
		}
		else if (pDesc->strPrototag.find(L"Grinder") != pDesc->strPrototag.npos)
		{
			m_strTextureTag = TEXT("Prototype_Component_Texture_Grinder");
		}

		Ready_Component(m_strTextureTag);
	}

	m_iItemIndex = pDesc->iItemIndex;
	m_isUsable = pDesc->isUsable;
	m_isSelect = pDesc->isSelect;
	
	if (pDesc->isConsumable)
	{
		m_strCaption = to_wstring(pDesc->iUseCount);
	}

	

}

HRESULT CIcon_Item::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (m_isSelect)
	{
		m_iTextureIndex = 1;
	}
	else
	{
		m_iTextureIndex = 0;
	}

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_iTextureIndex)))
		return E_FAIL;

	if (FAILED(m_pEffectTextureCom->Bind_ShaderResource(m_pShaderCom, "g_HighlightTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_Color", &m_vColor, sizeof(_float4))))
		return E_FAIL;

	// 기본값을 던지고, 추가로 덮어쓰자

	if (m_isUsable)
	{
		m_fCurrentAlpha = 1.f;
	}
	else
	{
		m_fCurrentAlpha = 0.5f;
	}

	if (FAILED(m_pShaderCom->Bind_RawValue("g_Alpha", &m_fCurrentAlpha, sizeof(_float))))
		return E_FAIL;


	for (auto& pFeature : m_pUIFeatures)
	{
		if (nullptr != pFeature)
			pFeature->Bind_ShaderResources(m_pShaderCom);
	}

	// 아이템 따라서 값 넘기기

	

	_bool isItemEmpty = {false};
	
	if (nullptr != m_pItemTextureCom)
	{
		if (FAILED(m_pItemTextureCom->Bind_ShaderResource(m_pShaderCom, "g_ItemTexture", m_iItemIndex)))
			return E_FAIL;

		isItemEmpty = true;
	}
	
	
	_float4  vIconDesc = { _float(m_isSelect) , _float(isItemEmpty), _float(m_isInput), m_fInputTime / m_fDuration};

	if (FAILED(m_pShaderCom->Bind_RawValue("g_ItemDesc", &vIconDesc, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pInputTextureCom->Bind_ShaderResource(m_pShaderCom, "g_InputTexture", 0)))
		return E_FAIL;

	return S_OK;
}

HRESULT CIcon_Item::Ready_Component(_wstring& strTextureTag)
{
	if (strTextureTag.empty())
		return S_OK;

	if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), strTextureTag,
		TEXT("Com_Texture_Item"), reinterpret_cast<CComponent**>(&m_pItemTextureCom))))
		return E_FAIL;
	return S_OK;
}

CIcon_Item* CIcon_Item::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CIcon_Item* pInstance = new CIcon_Item(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CIcon_Item");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CIcon_Item::Clone(void* pArg)
{
	CIcon_Item* pInstance = new CIcon_Item(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CIcon_Item");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CIcon_Item::Free()
{
	__super::Free();

	Safe_Release(m_pItemTextureCom);
	Safe_Release(m_pEffectTextureCom);
	Safe_Release(m_pInputTextureCom);
}
