#include "CameraSequence.h"

CCameraSequence::~CCameraSequence()
{

}

void CCameraSequence::Add(_int startFrame, _int endFrame,_int type)
{
    CAMERA_KEY newKey;
    newKey.startFrame = startFrame;
    newKey.endFrame = endFrame;

    newKey.type = type;

    switch (type)
    {
    case 0:
        newKey.color = IM_COL32(255, 255, 0, 255); 
        break;
    case 1:
        newKey.color = IM_COL32(0, 255, 255, 255);  
        break;
    case 2:
        newKey.color = IM_COL32(255, 0, 255, 255);  
        break;
    default:
        break;
    }

    // 기본 카메라 파라미터 초기화
    XMStoreFloat3(&newKey.position, XMVectorZero());
    XMStoreFloat3(&newKey.rotation, XMVectorZero());
    XMStoreFloat3(&newKey.offSetRotation, XMVectorZero());
    newKey.fFov = 60.f;

    //newKey.lookAtTarget = XMVectorZero();
    //newKey.bLookAt = false;

    // 보간 타입 초기화
    newKey.interpPosition = INTERPOLATION_CAMERA::NONE;
    newKey.interpRotation = INTERPOLATION_CAMERA::NONE;
    newKey.interpFov = INTERPOLATION_CAMERA::NONE;
    //newKey.interpLookAt = INTERPOLATION_CAMERA::NONE;

    // 추가
    m_vecKeys.push_back(newKey);
}

void CCameraSequence::Add_KeyFrame(_int type, _int keyFrame)
{
    switch (type)
    {
    case 0:
        m_vecPosKeyFrames.push_back(keyFrame);
        break;
    case 1:
        m_vecRotKeyFrames.push_back(keyFrame);
        break;
    case 2:
        m_vecFovKeyFrames.push_back(keyFrame);
        break;
    default:
        break;
    }
}

void CCameraSequence::Delete_KeyFrame(_int type, _int keyFrame)
{
    switch (type)
    {
    case 0:
    {
        auto it = find(m_vecPosKeyFrames.begin(), m_vecPosKeyFrames.end(), keyFrame);
        if (it != m_vecPosKeyFrames.end())
            m_vecPosKeyFrames.erase(it);
        break;
    }
    case 1:
    {
        auto it = find(m_vecRotKeyFrames.begin(), m_vecRotKeyFrames.end(), keyFrame);
        if (it != m_vecRotKeyFrames.end())
            m_vecRotKeyFrames.erase(it);
        break;
    }
    case 2:
    {
        auto it = find(m_vecFovKeyFrames.begin(), m_vecFovKeyFrames.end(), keyFrame);
        if (it != m_vecFovKeyFrames.end())
            m_vecFovKeyFrames.erase(it);
        break;
    }
    default:
        break;
    }


}

void CCameraSequence::Set_EndFrame(_int endFrame)
{
    for (auto& key : m_vecKeys)
        key.endFrame = endFrame;
}

void CCameraSequence::CustomDraw(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect&, const ImRect&, const ImRect&)
{
    const vector<_int>* pKeyFrames = nullptr;
    ImU32 color = IM_COL32(255, 255, 255, 255); // fallback color

    switch (index)
    {
    case 0:
        pKeyFrames = &m_vecPosKeyFrames;
        color = IM_COL32(255, 255, 0, 255); // Yellow
        break;
    case 1:
        pKeyFrames = &m_vecRotKeyFrames;
        color = IM_COL32(0, 255, 255, 255); // Cyan
        break;
    case 2:
        pKeyFrames = &m_vecFovKeyFrames;
        color = IM_COL32(255, 0, 255, 255); // Magenta
        break;
    default:
        return;
    }

    for (auto& keyFrame : *pKeyFrames)
    {
        _float frameNorm = (keyFrame - m_iFrameMin + 0.6f) / static_cast<_float>(m_iFrameMax - m_iFrameMin);

        _float x = rc.Min.x + frameNorm * (rc.Max.x - rc.Min.x);
        _float y = rc.Min.y;
        _float h = rc.Max.y - rc.Min.y;

        draw_list->AddLine(ImVec2(x, y), ImVec2(x, y + h), color);
        draw_list->AddCircleFilled(ImVec2(x, y + h / 2), 3.0f, color);
    }
}