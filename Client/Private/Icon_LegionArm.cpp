#include "Icon_LegionArm.h"
#include "GameInstance.h"

CIcon_LegionArm::CIcon_LegionArm(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CDynamic_UI{pDevice, pContext}
{
}

CIcon_LegionArm::CIcon_LegionArm(const CIcon_LegionArm& Prototype)
	:CDynamic_UI{Prototype}
{
}

HRESULT CIcon_LegionArm::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CIcon_LegionArm::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strProtoTag = TEXT("Prototype_GameObject_UI_Icon_Weapon");


	if (nullptr == pArg)
		return S_OK;


	return S_OK;
}

void CIcon_LegionArm::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CIcon_LegionArm::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CIcon_LegionArm::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CIcon_LegionArm::Render()
{
	__super::Render();

	return S_OK;
}

void CIcon_LegionArm::Update_ICon(_wstring& strTextureTag)
{
	// ³ªÁß¿¡ 
}

HRESULT CIcon_LegionArm::Bind_ShaderResources()
{
	__super::Bind_ShaderResources();

	return S_OK;
}

CIcon_LegionArm* CIcon_LegionArm::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CIcon_LegionArm* pInstance = new CIcon_LegionArm(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CIcon_LegionArm");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CIcon_LegionArm::Clone(void* pArg)
{
	CIcon_LegionArm* pInstance = new CIcon_LegionArm(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CIcon_LegionArm");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CIcon_LegionArm::Free()
{
	__super::Free();
}
