#include "UI_Feature_Fade.h"
#include "Shader.h"
#include "Dynamic_UI.h"

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
    if (nullptr == pArg)
        return S_OK;

    UI_FEATURE_FADE_DESC* pDesc = static_cast<UI_FEATURE_FADE_DESC*>(pArg);

    m_isLoop = pDesc->isLoop;
    m_iStartFrame = pDesc->iStartFrame;
    m_fStartAlpha = pDesc->fStartAlpha;
    m_fEndAlpha = pDesc->fEndAlpha;

   
    m_iEndFrame = pDesc->iEndFrame;

    m_iRange = m_iEndFrame - m_iStartFrame;


    m_strProtoTag = TEXT("Prototype_Component_UI_Feature_Fade");

    return S_OK;
}

void CUI_Feature_Fade::Update(_int& iCurrentFrame, CDynamic_UI* pUI,  _bool isReverse)
{
    // 나머지, loop면 나머지 연산해서 계속 프레임을 돌리고, 아니면 그냥 씀

    if (m_iRange <= 0)
        return;

    if (iCurrentFrame < m_iStartFrame)
    {
     
        return;
    }
        
    
        

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

    _float t = std::clamp(float(m_iCurrentFrame - m_iStartFrame) / m_iRange, 0.f, 1.f);

    if (!isReverse)
    {
        // 정방향
        m_fCurrentAlpha = LERP(m_fStartAlpha, m_fEndAlpha, t);
    }
    else
    {
        // 역방향 (LERP 순서 반전)
        m_fCurrentAlpha = LERP(m_fEndAlpha, m_fStartAlpha, t);
    }
  
    pUI->Set_Alpha(m_fCurrentAlpha);

}

HRESULT CUI_Feature_Fade::Bind_ShaderResources(CShader* pShader)
{

    return S_OK;
}

UI_FEATURE_TOOL_DESC CUI_Feature_Fade::Get_Desc_From_Tool()
{
    UI_FEATURE_TOOL_DESC eDesc = {};

    eDesc.isLoop = m_isLoop;
    eDesc.iType = 0;
    eDesc.iStartFrame = m_iStartFrame;
    eDesc.iEndFrame = m_iEndFrame;
    eDesc.fStartAlpha = m_fStartAlpha;
    eDesc.fEndAlpha = m_fEndAlpha;
    eDesc.strTypeTag = "Fade";


    return eDesc;
}

UI_FEATRE_DESC& CUI_Feature_Fade::Get_Desc()
{
   

    m_eDesc.isLoop = m_isLoop;
    m_eDesc.iStartFrame = m_iStartFrame;
    m_eDesc.iEndFrame = m_iEndFrame;
    m_eDesc.fStartAlpha = m_fStartAlpha;
    m_eDesc.fEndAlpha = m_fEndAlpha;
    m_eDesc.strProtoTag = "Prototype_Component_UI_Feature_Fade";

    return m_eDesc;
}

json CUI_Feature_Fade::Serialize()
{
    json j = __super::Serialize();


    j["StartAlpha"] = m_fStartAlpha;
   
    j["EndAlpha"] = m_fEndAlpha;

    j["FeatureProtoTag"] = "Prototype_Component_UI_Feature_Fade";


    return j;
}

void CUI_Feature_Fade::Deserialize(const json& j)
{
    __super::Deserialize(j);

    m_fStartAlpha = j["StartAlpha"].get<_float>();
    m_fEndAlpha = j["EndAlpha"].get<_float>();

    string strPrototag = j["FeatureProtoTag"];
    m_strProtoTag = StringToWStringU8(strPrototag);

    m_iRange = m_iEndFrame - m_iStartFrame;


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
