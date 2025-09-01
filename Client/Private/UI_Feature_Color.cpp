#include "UI_Feature_Color.h"

CUI_Feature_Color::CUI_Feature_Color(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CUI_Feature{pDevice, pContext}
{
}

CUI_Feature_Color::CUI_Feature_Color(const CUI_Feature_Color& Prototype)
    :CUI_Feature{ Prototype }
{
}

HRESULT CUI_Feature_Color::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_Feature_Color::Initialize(void* pArg)
{
    if (nullptr == pArg)
        return S_OK;

    UI_FEATURE_COLOR_DESC* pDesc = static_cast<UI_FEATURE_COLOR_DESC*>(pArg);

    m_isLoop = pDesc->isLoop;
    m_iStartFrame = pDesc->iStartFrame;
    m_fStartColor = pDesc->fStartColor;
    m_fEndColor = pDesc->fEndColor;


    m_iEndFrame = pDesc->iEndFrame;

    m_iRange = m_iEndFrame - m_iStartFrame;


    m_strProtoTag = TEXT("Prototype_Component_UI_Feature_Color");

    return S_OK;
}

void CUI_Feature_Color::Update(_int& iCurrentFrame, CDynamic_UI* pUI, _bool isReverse)
{
    if (m_iStartFrame == m_iEndFrame)
        return;

    if (iCurrentFrame < m_iStartFrame || (!m_isLoop && iCurrentFrame > m_iEndFrame))
        return;

    int frame = 0;
    if (m_isLoop) {
        frame = (iCurrentFrame - m_iStartFrame) % m_iRange;
    }
    else {
        frame = iCurrentFrame - m_iStartFrame;
        frame = std::clamp(frame, 0, m_iRange - 1);
    }

    // 0~1 범위로 t 계산
    float t = float(frame) / float(m_iRange - 1);

    if (isReverse)
        t = 1.f - t;

    m_fCurrentColor.x = LERP(m_fStartColor.x, m_fEndColor.x, t);
    m_fCurrentColor.y = LERP(m_fStartColor.y, m_fEndColor.y, t);
    m_fCurrentColor.z = LERP(m_fStartColor.z, m_fEndColor.z, t);
    m_fCurrentColor.w = LERP(m_fStartColor.w, m_fEndColor.w, t);

    pUI->Set_Color(m_fCurrentColor);
}

HRESULT CUI_Feature_Color::Bind_ShaderResources(CShader* pShader)
{

    return S_OK;
}

UI_FEATURE_TOOL_DESC CUI_Feature_Color::Get_Desc_From_Tool()
{
    UI_FEATURE_TOOL_DESC eDesc = {};

    eDesc.isLoop = m_isLoop;
    eDesc.iType = 2;
    eDesc.iStartFrame = m_iStartFrame;
    eDesc.iEndFrame = m_iEndFrame;
    eDesc.fStartColor = m_fStartColor;
    eDesc.fEndColor = m_fEndColor;
    eDesc.strTypeTag = "Color";

    return eDesc;
}

UI_FEATRE_DESC& CUI_Feature_Color::Get_Desc()
{
    // TODO: 여기에 return 문을 삽입합니다.

    m_eDesc.isLoop = m_isLoop;
    m_eDesc.iStartFrame = m_iStartFrame;
    m_eDesc.iEndFrame = m_iEndFrame;
    m_eDesc.fStartColor = m_fStartColor;
    m_eDesc.fEndColor = m_fEndColor;
    m_eDesc.strProtoTag = "Prototype_Component_UI_Feature_Color";
    

    return m_eDesc;
}

json CUI_Feature_Color::Serialize()
{
    json j = __super::Serialize();


    j["StartColor"]["X"] = m_fStartColor.x;
    j["StartColor"]["Y"] = m_fStartColor.y;
    j["StartColor"]["Z"] = m_fStartColor.z;
    j["StartColor"]["W"] = m_fStartColor.w;

    j["EndColor"]["X"] = m_fEndColor.x;
    j["EndColor"]["Y"] = m_fEndColor.y;
    j["EndColor"]["Z"] = m_fEndColor.z;
    j["EndColor"]["W"] = m_fEndColor.w;

    j["FeatureProtoTag"] = "Prototype_Component_UI_Feature_Color";


    return j;
}

void CUI_Feature_Color::Deserialize(const json& j)
{
    __super::Deserialize(j);

    string strPrototag = j["FeatureProtoTag"];
    m_strProtoTag = StringToWStringU8(strPrototag);

    m_fStartColor = { j["StartColor"]["X"] ,j["StartColor"]["Y"] ,j["StartColor"]["Z"] ,j["StartColor"]["W"] };
    m_fEndColor = { j["EndColor"]["X"] ,j["EndColor"]["Y"] ,j["EndColor"]["Z"] ,j["EndColor"]["W"] };
}


CUI_Feature_Color* CUI_Feature_Color::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_Feature_Color* pInstance = new CUI_Feature_Color(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_Feature_Color");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CUI_Feature_Color::Clone(void* pArg)
{
    CUI_Feature_Color* pInstance = new CUI_Feature_Color(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_Feature_Color");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_Feature_Color::Free()
{
    __super::Free();
}
