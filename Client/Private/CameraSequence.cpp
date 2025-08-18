#include "CameraSequence.h"

CCameraSequence::~CCameraSequence()
{

}

void CCameraSequence::InitAllFrames()
{
    m_vecMatrixPosKeyFrames.clear();
    m_vecOffSetPosKeyFrames.clear();
    m_vecOffSetRotKeyFrames.clear();
    m_vecFovKeyFrames.clear();
    m_vecTargetKeyFrames.clear();
}

void CCameraSequence::Add(_int startFrame, _int endFrame,_int type)
{
    CAMERA_KEY newKey;
    newKey.startFrame = startFrame;
    newKey.endFrame = endFrame;
    newKey.type = type;

    switch (type)
    {
    case 0: newKey.color = IM_COL32(240, 228, 66, 255); break; // Yellow
    case 1: newKey.color = IM_COL32(86, 180, 233, 255); break; // Sky Blue
    case 2: newKey.color = IM_COL32(0, 158, 115, 255); break; // Bluish Green
    case 3: newKey.color = IM_COL32(213, 94, 0, 255); break; // Vermillion/Orange
    case 4: newKey.color = IM_COL32(204, 121, 167, 255); break; // Reddish Purple
    default:
        break;
    }

    // 기본 카메라 파라미터 초기화
    XMStoreFloat3(&newKey.position, XMVectorZero());
    XMStoreFloat3(&newKey.rotation, XMVectorZero());
    newKey.interpWorldPos = INTERPOLATION_CAMERA::NONE;

    XMStoreFloat3(&newKey.offSetPosition, XMVectorZero());
    newKey.interpOffSetPos = INTERPOLATION_CAMERA::NONE;

    XMStoreFloat3(&newKey.offSetRotation, XMVectorZero());
    newKey.interpOffSetRot = INTERPOLATION_CAMERA::NONE;

    newKey.fFov = 60.f;
    newKey.interpFov = INTERPOLATION_CAMERA::NONE;

    newKey.eTarget = TARGET_CAMERA::NONE;
    newKey.fPitch = 0.f;
    newKey.fYaw = 0.f;
    newKey.fDistance = 0.f;

    // 추가
    m_vecKeys.push_back(newKey);
}

void CCameraSequence::Add_KeyFrame(_int type, _int keyFrame)
{
    switch (type)
    {
    case 0:
        m_vecMatrixPosKeyFrames.push_back(keyFrame);
        break;
    case 1:
        m_vecOffSetPosKeyFrames.push_back(keyFrame);
        break;
    case 2:
        m_vecOffSetRotKeyFrames.push_back(keyFrame);
        break;
    case 3:
        m_vecFovKeyFrames.push_back(keyFrame);
        break;
    case 4:
        m_vecTargetKeyFrames.push_back(keyFrame);
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
        auto it = find(m_vecMatrixPosKeyFrames.begin(), m_vecMatrixPosKeyFrames.end(), keyFrame);
        if (it != m_vecMatrixPosKeyFrames.end())
            m_vecMatrixPosKeyFrames.erase(it);
        break;
    }
    case 1:
    {
        auto it = find(m_vecOffSetPosKeyFrames.begin(), m_vecOffSetPosKeyFrames.end(), keyFrame);
        if (it != m_vecMatrixPosKeyFrames.end())
            m_vecOffSetPosKeyFrames.erase(it);
        break;
    }
    case 2:
    {
        auto it = find(m_vecOffSetRotKeyFrames.begin(), m_vecOffSetRotKeyFrames.end(), keyFrame);
        if (it != m_vecOffSetRotKeyFrames.end())
            m_vecOffSetRotKeyFrames.erase(it);
        break;
    }
    case 3:
    {
        auto it = find(m_vecFovKeyFrames.begin(), m_vecFovKeyFrames.end(), keyFrame);
        if (it != m_vecFovKeyFrames.end())
            m_vecFovKeyFrames.erase(it);
        break;
    }
    case 4:
    {
        auto it = find(m_vecTargetKeyFrames.begin(), m_vecTargetKeyFrames.end(), keyFrame);
        if (it != m_vecTargetKeyFrames.end())
            m_vecTargetKeyFrames.erase(it);
        break;
    }
    default:
        break;
    }


}

void CCameraSequence::Change_KeyFrame(_int type, _int originKeyFrame, _int changeKeyFrame)
{
    switch (type)
    {
    case 0:
    {
        auto it = find(m_vecMatrixPosKeyFrames.begin(), m_vecMatrixPosKeyFrames.end(), originKeyFrame);
        if (it != m_vecMatrixPosKeyFrames.end())
            *it = changeKeyFrame;
        break;
    }
    case 1:
    {
        auto it = find(m_vecOffSetPosKeyFrames.begin(), m_vecOffSetPosKeyFrames.end(), originKeyFrame);
        if (it != m_vecOffSetPosKeyFrames.end())
            *it = changeKeyFrame;
        break;
    }
    case 2:
    {
        auto it = find(m_vecOffSetRotKeyFrames.begin(), m_vecOffSetRotKeyFrames.end(), originKeyFrame);
        if (it != m_vecOffSetRotKeyFrames.end())
            *it = changeKeyFrame;
        break;
    }
    case 3:
    {
        auto it = find(m_vecFovKeyFrames.begin(), m_vecFovKeyFrames.end(), originKeyFrame);
        if (it != m_vecFovKeyFrames.end())
            *it = changeKeyFrame;
        break;
    }
    case 4:
    {
        auto it = find(m_vecTargetKeyFrames.begin(), m_vecTargetKeyFrames.end(), originKeyFrame);
        if (it != m_vecTargetKeyFrames.end())
            *it = changeKeyFrame;
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
    case 0: pKeyFrames = &m_vecMatrixPosKeyFrames; color = IM_COL32(240, 228, 66, 255); break; // Yellow
    case 1: pKeyFrames = &m_vecOffSetPosKeyFrames; color = IM_COL32(86, 180, 233, 255); break; // Sky Blue
    case 2: pKeyFrames = &m_vecOffSetRotKeyFrames; color = IM_COL32(0, 158, 115, 255); break; // Bluish Green
    case 3: pKeyFrames = &m_vecFovKeyFrames; color = IM_COL32(213, 94, 0, 255); break; // Vermillion/Orange
    case 4:  pKeyFrames = &m_vecTargetKeyFrames; color = IM_COL32(204, 121, 167, 255); break; // Reddish Purple
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