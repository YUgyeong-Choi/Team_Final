#include "GameInstance.h"

#include "DecalToolObject.h"
#include "Level_YW.h"

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

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	//���� ��� ����
	DECALTOOLOBJECT_DESC* pDesc = static_cast<DECALTOOLOBJECT_DESC*>(pArg);
	m_FilePath[ENUM_CLASS(TEXTURE_TYPE::ARMT)] = pDesc->FilePath[ENUM_CLASS(TEXTURE_TYPE::ARMT)];
	m_FilePath[ENUM_CLASS(TEXTURE_TYPE::N)] = pDesc->FilePath[ENUM_CLASS(TEXTURE_TYPE::N)];
	m_FilePath[ENUM_CLASS(TEXTURE_TYPE::BC)] = pDesc->FilePath[ENUM_CLASS(TEXTURE_TYPE::BC)];


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
	//�ø�? �ν��Ͻ�?

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_DECAL, this);
}

HRESULT CDecalToolObject::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;


	
#ifdef _DEBUG

	if (CLevel_YW::IMGUITOOL::DECAL == static_cast<CLevel_YW*>(m_pGameInstance->Get_CurrentLevel())->Get_AciveTool())
	{
		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;

		if (FAILED(m_pVIBufferCom->Bind_Buffers()))
			return E_FAIL;

		if (FAILED(m_pVIBufferCom->Render()))
			return E_FAIL;
	}

#endif // _DEBUG


	return S_OK;

}

HRESULT CDecalToolObject::Set_Texture(TEXTURE_TYPE eType, string TexturePath, string FileName)
{
	//������Ÿ���� �����
	wstring wstrPrototypeTag = TEXT("Prototype_Component_Texture_") + StringToWString(FileName);
	if (m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::YW), wstrPrototypeTag) == nullptr)
	{
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), wstrPrototypeTag,
			CTexture::Create(m_pDevice, m_pContext, StringToWString(TexturePath).c_str()))))
			return E_FAIL;
	}

	wstring wstrComTag = {};

	switch (eType)
	{
	case TEXTURE_TYPE::ARMT:
		wstrComTag = TEXT("Com_Texture_ARMT");
		break;
	case TEXTURE_TYPE::N:
		wstrComTag = TEXT("Com_Texture_N");
		break;
	case TEXTURE_TYPE::BC:
		wstrComTag = TEXT("Com_Texture_BC");
		break;
	}
	
	//���Ƴ���
	Remove_Component(wstrComTag);
	Safe_Release(m_pTextureCom[ENUM_CLASS(eType)]);
	/* For.Com_Texture_ARMT */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), wstrPrototypeTag,
		wstrComTag, reinterpret_cast<CComponent**>(&m_pTextureCom[ENUM_CLASS(eType)]))))
		return E_FAIL;

	//�ؽ��� ���� ����
	m_PrototypeTag[ENUM_CLASS(eType)] = wstrPrototypeTag;
	m_FilePath[ENUM_CLASS(eType)] = StringToWString(TexturePath);

	return S_OK;
}

HRESULT CDecalToolObject::Ready_Components(void* Arg)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Decal"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;
	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_VolumeMesh"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	DECAL_DESC* pDesc = static_cast<DECAL_DESC*>(Arg);

	//�ؽ��Ŀ����� ������ ���� ������ �⺻���� ����
	if (pDesc->PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::ARMT)].empty() ||
		pDesc->PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::N)].empty() ||
		pDesc->PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::BC)].empty())
	{
		//�⺻ ����Ʈ ��Į ������, ���߿� ������ �� ����Ʈ�� ���� ���ϰԲ� �ؾ���
		m_PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::ARMT)] = TEXT("Prototype_Component_Texture_DefaultDecal");
		m_PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::N)] = TEXT("Prototype_Component_Texture_DefaultDecal");
		m_PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::BC)] = TEXT("Prototype_Component_Texture_DefaultDecal");
	}
	else
	{
		//�ؽ��� ���ְ�
		m_PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::ARMT)] = pDesc->PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::ARMT)];
		m_PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::N)] = pDesc->PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::N)];
		m_PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::BC)] = pDesc->PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::BC)];

		////���� �н��� ����������
		//json ResourceJson;
		//{
		//	ifstream ResFile("../Bin/Save/DecalTool/Resource_TEST.json");
		//	if (ResFile.is_open())
		//		ResFile >> ResourceJson;
		//}

		//auto it = ResourceJson.find(WStringToString(m_PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::ARMT)]));
		//m_FilePath[ENUM_CLASS(TEXTURE_TYPE::ARMT)] = StringToWString(it.value());

		//it = ResourceJson.find(WStringToString(m_PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::N)]));
		//m_FilePath[ENUM_CLASS(TEXTURE_TYPE::N)] = StringToWString(it.value());

		//it = ResourceJson.find(WStringToString(m_PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::BC)]));
		//m_FilePath[ENUM_CLASS(TEXTURE_TYPE::BC)] = StringToWString(it.value());
	}

	/* For.Com_Texture_ARMT */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), m_PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::ARMT)],
		TEXT("Com_Texture_ARMT"), reinterpret_cast<CComponent**>(&m_pTextureCom[ENUM_CLASS(TEXTURE_TYPE::ARMT)]))))
		return E_FAIL;

	/* For.Com_Texture_N */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), m_PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::N)],
		TEXT("Com_Texture_N"), reinterpret_cast<CComponent**>(&m_pTextureCom[ENUM_CLASS(TEXTURE_TYPE::N)]))))
		return E_FAIL;

	/* For.Com_Texture_BC */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), m_PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::BC)],
		TEXT("Com_Texture_BC"), reinterpret_cast<CComponent**>(&m_pTextureCom[ENUM_CLASS(TEXTURE_TYPE::BC)]))))
		return E_FAIL;

	return S_OK;
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
