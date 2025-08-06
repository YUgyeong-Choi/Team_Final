#include "UI_Bar_Loading.h"
#include "GameInstance.h"
#include "UI_Feature.h"

CUI_Bar_Loading::CUI_Bar_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CDynamic_UI{pDevice, pContext}
{
}

CUI_Bar_Loading::CUI_Bar_Loading(const CUI_Bar_Loading& Prototype)
    :CDynamic_UI{Prototype}
{
}

HRESULT CUI_Bar_Loading::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_Bar_Loading::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

void CUI_Bar_Loading::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CUI_Bar_Loading::Update(_float fTimeDelta)
{
}

void CUI_Bar_Loading::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CUI_Bar_Loading::Render()
{
    __super::Render();

    return S_OK;
}

void CUI_Bar_Loading::Update_From_Feature_Ratio(atomic<_float>& fRatio)
{
	m_fRatio = fRatio.load();

	m_iCurrentFrame = _int(fRatio.load() * m_iRange);
	for (auto pFeature : m_pUIFeatures)
	{
		pFeature->Update(m_iCurrentFrame, this);
	}
}

CUI_Bar_Loading* CUI_Bar_Loading::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Bar_Loading* pInstance = new CUI_Bar_Loading(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Bar_Loading");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Bar_Loading::Clone(void* pArg)
{
	CUI_Bar_Loading* pInstance = new CUI_Bar_Loading(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_Bar_Loading");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Bar_Loading::Free()
{
	__super::Free();

	
}

