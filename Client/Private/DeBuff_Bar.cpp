#include "DeBuff_Bar.h"
#include "GameInstance.h"
#include "Observer_Player_Status.h"

CDeBuff_Bar::CDeBuff_Bar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CDynamic_UI{ pDevice, pContext }
{
}

CDeBuff_Bar::CDeBuff_Bar(const CDeBuff_Bar& Prototype)
    :CDynamic_UI{ Prototype }
{
}

HRESULT CDeBuff_Bar::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CDeBuff_Bar::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_strProtoTag = TEXT("Prototype_GameObject_UI_DeBuff_Bar");

    Ready_Component(m_strTextureTag);

 

    if (nullptr == pArg)
        return S_OK;

    return S_OK;
}

void CDeBuff_Bar::Priority_Update(_float fTimeDelta)
{
}

void CDeBuff_Bar::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CDeBuff_Bar::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CDeBuff_Bar::Render()
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

HRESULT CDeBuff_Bar::Bind_ShaderResources()
{
    __super::Bind_ShaderResources();

    // ³Ñ°ÜÁà

    if (FAILED(m_pBackTextureCom->Bind_ShaderResource(m_pShaderCom, "g_BackgroundTexture", 0)))
        return E_FAIL;

    if (FAILED(m_pGradationCom->Bind_ShaderResource(m_pShaderCom, "g_GradationTexture", 0)))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_BarRatio", &m_fRatio, sizeof(_float))))
        return E_FAIL;


    _float fGradation = 1.f;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_UseGradation", &fGradation, sizeof(_float))))
        return E_FAIL;

    fGradation = 0.f;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_IsDurablityBar", &fGradation, sizeof(_float))))
        return E_FAIL;

    return S_OK;
}

HRESULT CDeBuff_Bar::Ready_Component(const wstring& strTextureTag)
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
    if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Cloud"),
        TEXT("Com_Texture_Gradation"), reinterpret_cast<CComponent**>(&m_pGradationCom))))
        return E_FAIL;

    return S_OK;
}

CDeBuff_Bar* CDeBuff_Bar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CDeBuff_Bar* pInstance = new CDeBuff_Bar(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CDeBuff_Bar");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CDeBuff_Bar::Clone(void* pArg)
{
    CDeBuff_Bar* pInstance = new CDeBuff_Bar(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CDeBuff_Bar");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CDeBuff_Bar::Free()
{
    __super::Free();

    Safe_Release(m_pBackTextureCom);
    Safe_Release(m_pGradationCom);

}
