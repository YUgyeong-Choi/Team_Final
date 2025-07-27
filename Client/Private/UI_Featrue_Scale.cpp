#include "UI_Featrue_Scale.h"
#include "Shader.h"

CUI_Feature_Scale::CUI_Feature_Scale(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)	
	:CUI_Feature{pDevice, pContext}
{
}

CUI_Feature_Scale::CUI_Feature_Scale(const CUI_Feature_Scale& Prototype)
	:CUI_Feature{Prototype}
{
}

HRESULT CUI_Feature_Scale::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Feature_Scale::Initialize(void* pArg)
{
	UI_FEATURE_SCALE_DESC* pDesc = static_cast<UI_FEATURE_SCALE_DESC*>(pArg);

	m_isLoop = pDesc->isLoop;
	m_iStartFrame = pDesc->iStartFrame;
	m_iEndFrame = pDesc->iEndFrame;
	m_fStartScale = pDesc->fStartScale;
	m_fEndScale = pDesc->fEndScale;

	

	m_iRange = m_iEndFrame - m_iStartFrame;

	return S_OK;
	
}

void CUI_Feature_Scale::Update(_int& iCurrentFrame, CDynamic_UI* pUI)
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

   
    m_fCurrentScale.x = LERP( m_fStartScale.x, m_fEndScale.x, t);
    m_fCurrentScale.y = LERP( m_fStartScale.y, m_fEndScale.y, t);
   

    m_fCurrentScale.x *= g_iWinSizeX;
    m_fCurrentScale.y *= g_iWinSizeY;

    pUI->Get_TransfomCom()->Scaling(m_fCurrentScale.x, m_fCurrentScale.y);


  
}

HRESULT CUI_Feature_Scale::Bind_ShaderResources(CShader* pShader)
{
	
	return S_OK;
}

UI_FEATURE_TOOL_DESC CUI_Feature_Scale::Get_Desc_From_Tool()
{
    UI_FEATURE_TOOL_DESC eDesc = {};

    eDesc.isLoop = m_isLoop;
    eDesc.iType = 3;
    eDesc.iStartFrame = m_iStartFrame;
    eDesc.iEndFrame = m_iEndFrame;
    eDesc.fStartScale = m_fStartScale;
    eDesc.fEndScale = m_fEndScale;
    eDesc.strTypeTag = "Scale";

    return eDesc;
}

UI_FEATRE_DESC& CUI_Feature_Scale::Get_Desc()
{
	
    m_eDesc.isLoop = m_isLoop;
    m_eDesc.iStartFrame = m_iStartFrame;
    m_eDesc.iEndFrame = m_iEndFrame;
    m_eDesc.fStartScale = m_fStartScale;
    m_eDesc.fEndScale = m_fEndScale;
    m_eDesc.strProtoTag = "Prototype_Component_UI_Feature_Scale";

	return m_eDesc;
}

CUI_Feature_Scale* CUI_Feature_Scale::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_Feature_Scale* pInstance = new CUI_Feature_Scale(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_Feature_Scale");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CUI_Feature_Scale::Clone(void* pArg)
{
    CUI_Feature_Scale* pInstance = new CUI_Feature_Scale(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_Feature_Scale");
        Safe_Release(pInstance);
    }

    return pInstance;
}
void CUI_Feature_Scale::Free()
{
    __super::Free();
}