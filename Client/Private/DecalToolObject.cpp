#include "GameInstance.h"

#include "DecalToolObject.h"

CDecalToolObject::CDecalToolObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CDecal(pDevice, pContext)
{
}

CDecalToolObject::CDecalToolObject(const CDecalToolObject& Prototype)
	:CDecal(Prototype)
{
}

HRESULT CDecalToolObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDecalToolObject::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CDecalToolObject::Priority_Update(_float fTimeDelta)
{
}

void CDecalToolObject::Update(_float fTimeDelta)
{
}

void CDecalToolObject::Late_Update(_float fTimeDelta)
{
	//컬링? 인스턴싱?

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_DECAL, this);
}

HRESULT CDecalToolObject::Render()
{
	return __super::Render();

}

HRESULT CDecalToolObject::Set_ARM_Texture(string TexturePath, string FileName)
{
	//프로토타입을 만들고
	wstring wstrPrototypeTag = TEXT("Prototype_Component_Texture_") + StringToWString(FileName);
	if (m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::YW), wstrPrototypeTag) == nullptr)
	{
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), wstrPrototypeTag,
			CTexture::Create(m_pDevice, m_pContext, StringToWString(TexturePath).c_str()))))
			return E_FAIL;
	}

	//갈아끼자
	Remove_Component(TEXT("Com_Texture_ARMT"));
	Safe_Release(m_pTextureCom[ENUM_CLASS(TEXTURE_TYPE::ARMT)]);
	/* For.Com_Texture_ARMT */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), wstrPrototypeTag,
		TEXT("Com_Texture_ARMT"), reinterpret_cast<CComponent**>(&m_pTextureCom[ENUM_CLASS(TEXTURE_TYPE::ARMT)]))))
		return E_FAIL;

	return S_OK;
}

HRESULT CDecalToolObject::Ready_Components()
{
	return __super::Ready_Components();
}

HRESULT CDecalToolObject::Bind_ShaderResources()
{
	return __super::Bind_ShaderResources();
}

CDecalToolObject* CDecalToolObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CDecalToolObject* pInstance = new CDecalToolObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CDecalToolObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDecalToolObject::Clone(void* pArg)
{
	CDecalToolObject* pInstance = new CDecalToolObject(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CDecalToolObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CDecalToolObject::Free()
{
	__super::Free();
}
