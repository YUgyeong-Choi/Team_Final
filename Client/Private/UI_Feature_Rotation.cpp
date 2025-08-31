#include "UI_Feature_Rotation.h"

CUI_Feature_Rotation::CUI_Feature_Rotation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CUI_Feature{pDevice, pContext}
{
}

CUI_Feature_Rotation::CUI_Feature_Rotation(const CUI_Feature_Rotation& Prototype)
    :CUI_Feature{Prototype}
{
}

HRESULT CUI_Feature_Rotation::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_Feature_Rotation::Initialize(void* pArg)
{
    if (nullptr == pArg)
        return S_OK;

    UI_FEATURE_ROTATION_DESC* pDesc = static_cast<UI_FEATURE_ROTATION_DESC*>(pArg);

    m_isLoop = pDesc->isLoop;
    m_iStartFrame = pDesc->iStartFrame;
    m_fStartRotation = pDesc->fStartRotation;
    m_fEndRotation = pDesc->fEndRotation;

    m_fRotationPos = pDesc->fRotationPos;


    m_iEndFrame = pDesc->iEndFrame;

    m_iRange = m_iEndFrame - m_iStartFrame;

    m_strProtoTag = TEXT("Prototype_Component_UI_Feature_Rotation");


    m_vInitPos = { pDesc->fInitPos.x * g_iWinSizeX - 0.5f * g_iWinSizeX, 0.5f * g_iWinSizeY - pDesc->fInitPos.y * g_iWinSizeY, 0.f, 1.f };
    m_vCenter = { m_fRotationPos.x * g_iWinSizeX - 0.5f * g_iWinSizeX, 0.5f * g_iWinSizeY - m_fRotationPos.y * g_iWinSizeY, 0.f, 1.f };
    m_vOffset =   m_vCenter - m_vInitPos;

    return S_OK;
}

void CUI_Feature_Rotation::Update(_int& iCurrentFrame, CDynamic_UI* pUI, _bool isReverse)
{

    if (m_iStartFrame == m_iEndFrame)
        return;

    if (iCurrentFrame < m_iStartFrame)
        return;

    if (m_isLoop)
    {
        m_iCurrentFrame = (iCurrentFrame - m_iStartFrame) % m_iRange;
    }
    else
    {
        m_iCurrentFrame = iCurrentFrame - m_iStartFrame;
        if (m_iCurrentFrame > m_iRange)
            return;
    }

    // 0 ~ 1 구간으로 보간 값 계산
    float t = std::clamp(float(m_iCurrentFrame) / m_iRange, 0.f, 1.f);

   
    // 방향에 따라 보간 순서 변경
    if (!isReverse)
    {
       
        m_fCurrentRotation = LERP(m_fEndRotation, m_fStartRotation, t);
      
    }
    else
    {
     
        m_fCurrentRotation = LERP(m_fStartRotation, m_fEndRotation, t);
    
    }


    _matrix matRotate = XMMatrixRotationZ(XMConvertToRadians(m_fCurrentRotation));


   

    _vector vScale = pUI->Get_TransfomCom()->Get_Scale();
    _matrix mScale = XMMatrixScalingFromVector(vScale);

    _vector vRotatedOffset = XMVector3TransformCoord(m_vOffset, matRotate);
    _vector vFinalPos = m_vCenter + vRotatedOffset;

    _matrix mWorld = mScale* matRotate * XMMatrixTranslationFromVector(vFinalPos);
    pUI->Get_TransfomCom()->Set_WorldMatrix(mWorld);


}

HRESULT CUI_Feature_Rotation::Bind_ShaderResources(CShader* pShader)
{
    return S_OK;
}

UI_FEATURE_TOOL_DESC CUI_Feature_Rotation::Get_Desc_From_Tool()
{
    UI_FEATURE_TOOL_DESC eDesc = {};

    eDesc.isLoop = m_isLoop;
    eDesc.iType = 4;
    eDesc.iStartFrame = m_iStartFrame;
    eDesc.iEndFrame = m_iEndFrame;
    eDesc.fStartAlpha = m_fStartRotation;
    eDesc.fEndAlpha = m_fEndRotation;
    eDesc.strTypeTag = "Rotation";

    return eDesc;
}

UI_FEATRE_DESC& CUI_Feature_Rotation::Get_Desc()
{
    m_eDesc.isLoop = m_isLoop;
    m_eDesc.iStartFrame = m_iStartFrame;
    m_eDesc.iEndFrame = m_iEndFrame;
    m_eDesc.fStartRotation = m_fStartRotation;
    m_eDesc.fEndRotation = m_fEndRotation;
    m_eDesc.fRotationPos = m_fRotationPos;
    m_eDesc.strProtoTag = "Prototype_Component_UI_Feature_Rotation";
    m_eDesc.fInitPos = { m_vInitPos.m128_f32[0], m_vInitPos.m128_f32[1] };

    return m_eDesc;
}

json CUI_Feature_Rotation::Serialize()
{
    json j = __super::Serialize();


    j["StartRotation"] = m_fStartRotation;

    j["EndRotation"] = m_fEndRotation;

    j["RotationPos"]["X"] = m_fRotationPos.x;
    j["RotationPos"]["Y"] = m_fRotationPos.y;

    j["Initpos"]["X"] = m_fInitPos.x;
    j["Initpos"]["Y"] = m_fInitPos.y;

    j["FeatureProtoTag"] = "Prototype_Component_UI_Feature_Rotation";


    return j;
}

void CUI_Feature_Rotation::Deserialize(const json& j)
{
    __super::Deserialize(j);

    m_fStartRotation = j["StartRotation"].get<_float>();
    m_fEndRotation = j["EndRotation"].get<_float>();

    m_fRotationPos = { j["RotationPos"]["X"] , j["RotationPos"]["Y"] };

    m_fInitPos = { j["Initpos"]["X"],  j["Initpos"]["Y"] };

    string strPrototag = j["FeatureProtoTag"];
    m_strProtoTag = StringToWStringU8(strPrototag);


    m_vInitPos = { m_fInitPos.x * g_iWinSizeX - 0.5f * g_iWinSizeX, 0.5f * g_iWinSizeY - m_fInitPos.y * g_iWinSizeY, 0.f, 1.f };
    m_vCenter = { m_fRotationPos.x * g_iWinSizeX - 0.5f * g_iWinSizeX, 0.5f * g_iWinSizeY - m_fRotationPos.y * g_iWinSizeY, 0.f, 1.f };
    m_vOffset = m_vInitPos - m_vCenter  ;
}




CUI_Feature_Rotation* CUI_Feature_Rotation::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_Feature_Rotation* pInstance = new CUI_Feature_Rotation(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_Feature_Rotation");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CUI_Feature_Rotation::Clone(void* pArg)
{
    CUI_Feature_Rotation* pInstance = new CUI_Feature_Rotation(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_Feature_Rotation");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_Feature_Rotation::Free()
{
    __super::Free();

}
