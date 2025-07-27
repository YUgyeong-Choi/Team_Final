#include "UI_Feature_Position.h"

CUI_Feature_Position::CUI_Feature_Position(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CUI_Feature{pDevice, pContext}
{
}

CUI_Feature_Position::CUI_Feature_Position(const CUI_Feature_Position& Prototype)
    :CUI_Feature{Prototype}
{
}

HRESULT CUI_Feature_Position::Initialize_Prototype()
{


    return S_OK;
}

HRESULT CUI_Feature_Position::Initialize(void* pArg)
{
    UI_FEATURE_POS_DESC* pDesc = static_cast<UI_FEATURE_POS_DESC*>(pArg);

    m_isLoop = pDesc->isLoop;
    m_iStartFrame = pDesc->iStartFrame;
    m_iEndFrame = pDesc->iEndFrame;
    m_fStartPos = pDesc->fStartPos;
    m_fEndPos = pDesc->fEndPos;

    m_fCurrentPos = m_fStartPos;

    m_iRange = m_iEndFrame - m_iStartFrame;

    return S_OK;
}

void CUI_Feature_Position::Update(_int& iCurrentFrame, CDynamic_UI* pUI)
{
    if (m_iStartFrame == m_iEndFrame)
        return;

    if (iCurrentFrame < m_iStartFrame)
        return;

    if (m_isLoop)
    {
        m_iCurrentFrame = iCurrentFrame % m_iRange;
    }
    else
    {
        m_iCurrentFrame = iCurrentFrame;

        if (m_iCurrentFrame >= m_iEndFrame)
            return;

    }

    _float t = std::clamp(float(m_iCurrentFrame) / m_iRange, 0.f, 1.f);

    if (!m_isReverse)
    {
        m_fCurrentPos.x = LERP(m_fStartPos.x, m_fEndPos.x, t);
        m_fCurrentPos.y = LERP(m_fStartPos.y, m_fEndPos.y, t);


    }
    else
    {
        m_fCurrentPos.x = LERP(m_fEndPos.x, m_fStartPos.x, t);
        m_fCurrentPos.y = LERP(m_fEndPos.y, m_fStartPos.y, t);
    }
   

    m_fCurrentPos.x *= g_iWinSizeX;
    m_fCurrentPos.y *= g_iWinSizeY;


    pUI->Get_TransfomCom()->Set_State(STATE::POSITION, XMVectorSet(m_fCurrentPos.x - g_iWinSizeX * 0.5f, -m_fCurrentPos.y + g_iWinSizeY * 0.5f, pUI->Get_Depth(), 1.f));


}

HRESULT CUI_Feature_Position::Bind_ShaderResources(CShader* pShader)
{
    // 생각나면 추가
    return  S_OK;
}

UI_FEATURE_TOOL_DESC CUI_Feature_Position::Get_Desc_From_Tool()
{

    UI_FEATURE_TOOL_DESC eDesc = {};

    eDesc.isLoop = m_isLoop;
    eDesc.iType = 2;
    eDesc.iStartFrame = m_iStartFrame;
    eDesc.iEndFrame = m_iEndFrame;
    eDesc.fStartPos = m_fStartPos;
    eDesc.fEndPos = m_fEndPos;
    eDesc.strTypeTag = "Pos";

    return eDesc;
}

UI_FEATRE_DESC& CUI_Feature_Position::Get_Desc()
{

    m_eDesc.isLoop = m_isLoop;
    m_eDesc.iStartFrame = m_iStartFrame;
    m_eDesc.iEndFrame = m_iEndFrame;
    m_eDesc.fStartPos = m_fStartPos;
    m_eDesc.fEndPos = m_fEndPos;
    m_eDesc.strProtoTag = "Prototype_Component_UI_Feature_Pos";

    return m_eDesc;
}

CUI_Feature_Position* CUI_Feature_Position::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_Feature_Position* pInstance = new CUI_Feature_Position(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_Feature_Position");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CUI_Feature_Position::Clone(void* pArg)
{
    CUI_Feature_Position* pInstance = new CUI_Feature_Position(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_Feature_Position");
        Safe_Release(pInstance);
    }

    return pInstance;
}
void CUI_Feature_Position::Free()
{
    __super::Free();
}
