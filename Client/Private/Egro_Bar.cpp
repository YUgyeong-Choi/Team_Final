#include "Egro_Bar.h"
#include "GameInstance.h"

CErgo_Bar::CErgo_Bar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)  
    :CDynamic_UI{pDevice, pContext}
{
}

CErgo_Bar::CErgo_Bar(const CErgo_Bar& Prototype)    
    :CDynamic_UI{Prototype}
{
}

HRESULT CErgo_Bar::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CErgo_Bar::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_strProtoTag = TEXT("Prototype_GameObject_UI_Ergo_Bar");

    // �ݹ��� ���

    Ready_Component(m_strTextureTag);

    if (nullptr == pArg)
        return S_OK;

    return S_OK;
}

void CErgo_Bar::Priority_Update(_float fTimeDelta)
{
}

void CErgo_Bar::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CErgo_Bar::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CErgo_Bar::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Begin(D_UI_DURABILITYBAR)))
        return E_FAIL;

    if (FAILED(m_pVIBufferCom->Bind_Buffers()))
        return E_FAIL;

    if (FAILED(m_pVIBufferCom->Render()))
        return E_FAIL;


    return S_OK;
}

HRESULT CErgo_Bar::Bind_ShaderResources()
{
    __super::Bind_ShaderResources();

    // �Ѱ���

    if (FAILED(m_pBackTextureCom->Bind_ShaderResource(m_pShaderCom, "g_BackgroundTexture", 0)))
        return E_FAIL;

    if (FAILED(m_pGradationCom->Bind_ShaderResource(m_pShaderCom, "g_GradationTexture", 0)))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_BarRatio", &m_fRatio, sizeof(_float))))
        return E_FAIL;

    _float fGradation = 0.f;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_UseGradation", &fGradation, sizeof(_float))))
        return E_FAIL;


    if (FAILED(m_pShaderCom->Bind_RawValue("g_IsDurablityBar", &fGradation, sizeof(_float))))
        return E_FAIL;

    return S_OK;
}

HRESULT CErgo_Bar::Ready_Component(const wstring& strTextureTag)
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
    if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Gradation_Right"),
        TEXT("Com_Texture_Gradation"), reinterpret_cast<CComponent**>(&m_pGradationCom))))
        return E_FAIL;

    return S_OK;
}

CErgo_Bar* CErgo_Bar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CErgo_Bar* pInstance = new CErgo_Bar(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CErgo_Bar");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CErgo_Bar::Clone(void* pArg)
{
    CErgo_Bar* pInstance = new CErgo_Bar(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CErgo_Bar");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CErgo_Bar::Free()
{
    __super::Free();

    Safe_Release(m_pBackTextureCom);
    Safe_Release(m_pGradationCom);

}
