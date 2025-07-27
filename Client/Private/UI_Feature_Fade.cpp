#include "UI_Feature_Fade.h"
#include "Shader.h"

CUI_Feature_Fade::CUI_Feature_Fade(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CUI_Feature{pDevice, pContext}
{
}

CUI_Feature_Fade::CUI_Feature_Fade(const CUI_Feature_Fade& Prototype)
    :CUI_Feature{Prototype}
{
}

HRESULT CUI_Feature_Fade::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_Feature_Fade::Initialize(void* pArg)
{
    UI_FEATURE_FADE_DESC* pDesc = static_cast<UI_FEATURE_FADE_DESC*>(pArg);

    m_isLoop = pDesc->isLoop;
    m_iStartFrame = pDesc->iStartFrame;
    m_fStartAlpha = pDesc->fStartAlpha;
    m_fEndAlpha = pDesc->fEndAlpha;
    m_fFadeTime = pDesc->fTime;

    // 지속 시간에 따라 끝나는 프레임을 늘린다.
    // 일단 1초에 60프레임으로 하도록 하고, 나중에 필요하면 수정
    m_iEndFrame = m_iStartFrame + int(m_fFadeTime * 60.f);

    m_iRange = int(m_fFadeTime * 60.f);

    return S_OK;
}

void CUI_Feature_Fade::Update(_int& iCurrentFrame, CDynamic_UI* pUI)
{
    // 나머지, loop면 나머지 연산해서 계속 프레임을 돌리고, 아니면 그냥 씀

    if (m_iRange <= 0)
        return;

    if (iCurrentFrame < m_iStartFrame)
        return;
   
    if (m_isLoop)
    {
         m_iCurrentFrame = (iCurrentFrame - m_iStartFrame) % m_iRange;    
    }
    else
    {
        m_iCurrentFrame = iCurrentFrame;
    }

 
    if (m_iCurrentFrame > m_iEndFrame)
    {
        return;
    }

    _float t = std::clamp(float(m_iCurrentFrame) / m_iRange, 0.f, 1.f);
    m_fCurrentAlpha = LERP(m_fStartAlpha, m_fEndAlpha, t);
    
    
  

 

}

HRESULT CUI_Feature_Fade::Bind_ShaderResources(CShader* pShader)
{
    if (FAILED(pShader->Bind_RawValue("g_Alpha", &m_fCurrentAlpha, sizeof(_float))))
        return E_FAIL;

    return S_OK;
}

UI_FEATURE_TOOL_DESC CUI_Feature_Fade::Get_Desc_From_Tool()
{
    UI_FEATURE_TOOL_DESC eDesc = {};

    eDesc.isLoop = m_isLoop;
    eDesc.iType = 0;
    eDesc.iStartFrame = m_iStartFrame;
    eDesc.iEndFrame = m_iEndFrame;
    eDesc.fTime = m_fFadeTime;
    eDesc.fStartAlpha = m_fStartAlpha;
    eDesc.fEndAlpha = m_fEndAlpha;


    return eDesc;
}

UI_FEATRE_DESC& CUI_Feature_Fade::Get_Desc()
{
   

    m_eDesc.isLoop = m_isLoop;
    m_eDesc.iStartFrame = m_iStartFrame;
    m_eDesc.iEndFrame = m_iEndFrame;
    m_eDesc.fTime = m_fFadeTime;
    m_eDesc.fStartAlpha = m_fStartAlpha;
    m_eDesc.fEndAlpha = m_fEndAlpha;
    m_eDesc.strProtoTag = "Prototype_Component_UI_Feature_Fade";

    return m_eDesc;
}

CUI_Feature_Fade* CUI_Feature_Fade::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_Feature_Fade* pInstance = new CUI_Feature_Fade(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_Feature_Fade");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CUI_Feature_Fade::Clone(void* pArg)
{
    CUI_Feature_Fade* pInstance = new CUI_Feature_Fade(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_Feature_Fade");
        Safe_Release(pInstance);
    }

    return pInstance;
}
void CUI_Feature_Fade::Free()
{
    __super::Free();
}
