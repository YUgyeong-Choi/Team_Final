#include "Legion_Bar.h"
#include "GameInstance.h"
#include "Observer_Weapon.h"

CLegion_Bar::CLegion_Bar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CDynamic_UI{pDevice, pContext}
{
}

CLegion_Bar::CLegion_Bar(const CLegion_Bar& Prototype)
    :CDynamic_UI{Prototype}
{
}

HRESULT CLegion_Bar::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CLegion_Bar::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_strProtoTag = TEXT("Prototype_GameObject_UI_Durability_Bar");

    // 콜백을 등록

    if (nullptr == m_pGameInstance->Find_Observer(TEXT("Weapon_Status")))
    {

        m_pGameInstance->Add_Observer(TEXT("Weapon_Status"), new CObserver_Weapon);

    }

    m_pGameInstance->Register_PushCallback(TEXT("Weapon_Status"), [this](const _wstring& eventType, void* data) {
        if (L"Legion" == eventType)
        {
            m_fLegion = *static_cast<_float*>(data);


        }
        else if (L"MaxLegion" == eventType)
        {
            m_fMaxLegion = *static_cast<_float*>(data);

            //m_iDurablity = m_iMaxDurablity;
        }
        else if (L"AddLegion" == eventType)
        {
            m_fLegion += *static_cast<_float*>(data);

            if (m_fLegion >= m_fMaxLegion)
                m_fLegion = m_fMaxLegion;

            if (m_fLegion <= 0.f)
                m_fLegion = 0.f;
        }

        m_fRatio = (m_fLegion) / m_fMaxLegion;

        });

    Ready_Component(m_strTextureTag);

    m_fRatio = 1.f;

    if (nullptr == pArg)
        return S_OK;

    return S_OK;
}

void CLegion_Bar::Priority_Update(_float fTimeDelta)
{
}

void CLegion_Bar::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CLegion_Bar::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CLegion_Bar::Render()
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

HRESULT CLegion_Bar::Bind_ShaderResources()
{
    __super::Bind_ShaderResources();

    // 넘겨줘

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

HRESULT CLegion_Bar::Ready_Component(const wstring& strTextureTag)
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

CLegion_Bar* CLegion_Bar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLegion_Bar* pInstance = new CLegion_Bar(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CLegion_Bar");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CLegion_Bar::Clone(void* pArg)
{
    CLegion_Bar* pInstance = new CLegion_Bar(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CLegion_Bar");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLegion_Bar::Free()
{
    __super::Free();

    Safe_Release(m_pBackTextureCom);
    Safe_Release(m_pGradationCom);

}
