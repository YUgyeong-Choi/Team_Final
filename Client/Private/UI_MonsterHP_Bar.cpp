#include "UI_MonsterHP_Bar.h"
#include "GameInstance.h"
#include "Camera_Manager.h"

CUI_MonsterHP_Bar::CUI_MonsterHP_Bar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CUIObject{ pDevice, pContext }
{
}

CUI_MonsterHP_Bar::CUI_MonsterHP_Bar(const CUI_MonsterHP_Bar& Prototype)
    :CUIObject{ Prototype }
{
}

HRESULT CUI_MonsterHP_Bar::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_MonsterHP_Bar::Initialize(void* pArg)
{
    HPBAR_DESC* pDesc = static_cast<HPBAR_DESC*>(pArg);


    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pHP = pDesc->pHP;

    m_isGroggy = pDesc->pIsGroggy;

    m_fMaxHP = *m_pHP;

    m_pParentMatrix = pDesc->pParentMatrix;
    
    _vector vPos = { 0.f,pDesc->fHeight,0.f,1.f };

    m_pTransformCom->Set_State(STATE::POSITION, vPos);

    m_pTransformCom->Scaling(0.08f, 0.025f, 1.f);

   


    return S_OK;
}

void CUI_MonsterHP_Bar::Priority_Update(_float fTimeDelta)
{
    if (m_bDead)
        return;
}

void CUI_MonsterHP_Bar::Update(_float fTimeDelta)
{
    if (m_fRenderTime > 0.f)
    {
        m_fRenderTime -= fTimeDelta;
    }
    else if(m_fRenderTime < 0.f)
    {
        m_fRenderTime = 0.f;
        m_fDamage = 0.f;
    }
       
}

void CUI_MonsterHP_Bar::Late_Update(_float fTimeDelta)
{
    if (m_fRenderTime > 0.f)
    {
        XMFLOAT4X4 parentMatrix = *m_pParentMatrix;


        parentMatrix._11 = 1.0f; parentMatrix._12 = 0.0f; parentMatrix._13 = 0.0f;
        parentMatrix._21 = 0.0f; parentMatrix._22 = 1.0f; parentMatrix._23 = 0.0f;
        parentMatrix._31 = 0.0f; parentMatrix._32 = 0.0f; parentMatrix._33 = 1.0f;
        // 위치는 유지
        // parentMatrix._41, _42, _43 는 그대로 둠

        // 결합
        XMStoreFloat4x4(
            &m_CombinedWorldMatrix,
            XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr()) * XMLoadFloat4x4(&parentMatrix)
        );

        // 
        _vector camPos = XMLoadFloat4(m_pGameInstance->Get_CamPosition());
        _vector vWorldPos = { m_CombinedWorldMatrix._41, m_CombinedWorldMatrix._42, m_CombinedWorldMatrix._43, 1.f };

        _float vDist = XMVectorGetX(XMVector3Length(camPos - vWorldPos));

        _matrix ViewMat = m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW);
        _matrix ProjMat = m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ);

        _vector vClipPos = XMVector4Transform(vWorldPos, ViewMat * ProjMat);

        vClipPos.m128_f32[0] /= vClipPos.m128_f32[3];
        vClipPos.m128_f32[1] /= vClipPos.m128_f32[3];
        vClipPos.m128_f32[2] /= vClipPos.m128_f32[3];

        _vector vScale = m_pTransformCom->Get_Scale();

        m_fX = (vClipPos.m128_f32[0] * 0.5f + 0.5f) * g_iWinSizeX;
        m_fY = (1.f - (vClipPos.m128_f32[1] * 0.5f + 0.5f)) * g_iWinSizeY;


        XMFLOAT4X4 world{};
        world._11 = vScale.m128_f32[0] * g_iWinSizeX;  // 픽셀 단위 스케일
        world._22 = vScale.m128_f32[1] * g_iWinSizeY;
        world._33 = 1.f;
        world._44 = 1.f;
        world._41 = m_fX - 0.5f * g_iWinSizeX;
        world._42 = -m_fY + 0.5f * g_iWinSizeY;
        world._43 = 0.f;


        //  가까운게 그려질 수 있도록
        m_fOffset = vDist * 0.001f;

        


        XMStoreFloat4x4(&m_CombinedWorldMatrix, XMLoadFloat4x4(&world));



        m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_UI, this);
    }

   
}

HRESULT CUI_MonsterHP_Bar::Render()
{

    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Begin(D_UI_HPBAR_MONSTER)))
        return E_FAIL;

    if (FAILED(m_pVIBufferCom->Bind_Buffers()))
        return E_FAIL;

    if (FAILED(m_pVIBufferCom->Render()))
        return E_FAIL;

    // 직교 위치 구해서 받은 데미지 찍기
    
    if (m_fDamage > 0.f)
    {
        _float fDamage = floorf(m_fDamage);
        _wstring strDamage = to_wstring(fDamage);

        m_pGameInstance->Draw_Font_Righted(L"Font_Medium", strDamage.c_str(), { m_fX + g_iWinSizeX * 0.035f , m_fY - g_iWinSizeY * 0.02f }, {1.f,1.f,1.f,1.f},0.f,{0.f,0.f},0.7f,0.f);
    }
        
    
    return S_OK;
}

HRESULT CUI_MonsterHP_Bar::Ready_Components()
{
    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_DynamicUI"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
        return E_FAIL;
    /* For.Com_VIBuffer */
    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
        TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
        return E_FAIL;

    /* For.Com_Texture */
    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Bar_Border"),
        TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
        return E_FAIL;

    /* For.Com_Texture */
    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Bar_Gradation"),
        TEXT("Com_Texture_Gradation"), reinterpret_cast<CComponent**>(&m_pGradationCom))))
        return E_FAIL;
    

        /* For.Com_Texture */
        if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Bar_Paralyze"),
            TEXT("Com_Texture_Blur"), reinterpret_cast<CComponent**>(&m_pBlurTextureCom))))
            return E_FAIL;
    return S_OK;
}

HRESULT CUI_MonsterHP_Bar::Bind_ShaderResources()
{

    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;

    
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
        return E_FAIL;

    _float4 vColor = { 0.7f, 0.14f, 0.14f, 1.f };

    if (FAILED(m_pShaderCom->Bind_RawValue("g_Color", &vColor, sizeof(_float4))))
        return E_FAIL;

    _float fAlpha = 1.f;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_Alpha", &fAlpha, sizeof(_float))))
        return E_FAIL;


    if (FAILED(m_pGradationCom->Bind_ShaderResource(m_pShaderCom, "g_GradationTexture", 0)))
        return E_FAIL;

    if (FAILED(m_pBlurTextureCom->Bind_ShaderResource(m_pShaderCom, "g_HighlightTexture", 0)))
        return E_FAIL;

    _float fRatio = *m_pHP / (m_fMaxHP);

    if (FAILED(m_pShaderCom->Bind_RawValue("g_BarRatio", &(fRatio), sizeof(_float))))   
        return E_FAIL;

    _float fGroogy = _float(*m_isGroggy);

    //_float fGroogy = 0.f;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_Groggy", &(fGroogy), sizeof(_float))))
        return E_FAIL;

    return S_OK;
}

CUI_MonsterHP_Bar* CUI_MonsterHP_Bar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_MonsterHP_Bar* pInstance = new CUI_MonsterHP_Bar(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_MonsterHP_Bar");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_MonsterHP_Bar::Clone(void* pArg)
{
    CUI_MonsterHP_Bar* pInstance = new CUI_MonsterHP_Bar(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_MonsterHP_Bar");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_MonsterHP_Bar::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pTextureCom);
    Safe_Release(m_pBlurTextureCom);
    Safe_Release(m_pGradationCom);
    Safe_Release(m_pVIBufferCom);

}
