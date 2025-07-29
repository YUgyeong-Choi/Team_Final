#include "CameraSequence.h"

CCameraSequence::~CCameraSequence()
{

}

void CCameraSequence::Add(_int startFrame, _int endFrame)
{
    CAMERA_KEY newKey;
    newKey.startFrame = startFrame;
    newKey.endFrame = endFrame;

    newKey.type = 0;
    newKey.color = IM_COL32(255, 200, 0, 255);

    // 기본 카메라 파라미터 초기화
    XMStoreFloat3(&newKey.position, XMVectorZero());
    XMStoreFloat3(&newKey.rotation, XMVectorZero());
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

void CCameraSequence::Add_KeyFrame(_int keyFrame)
{
    m_vecKeyFrames.push_back(keyFrame);
}

void CCameraSequence::Delete_KeyFrame(_int keyFrame)
{
    auto it = std::find(m_vecKeyFrames.begin(), m_vecKeyFrames.end(), keyFrame);
    if (it != m_vecKeyFrames.end())
        m_vecKeyFrames.erase(it);
}

void CCameraSequence::Set_EndFrame(_int endFrame)
{
    for (auto& key : m_vecKeys)
        key.endFrame = endFrame;
}

void CCameraSequence::CustomDraw(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect&, const ImRect&, const ImRect&)
{

    for (auto& keyFrame : m_vecKeyFrames)
    {
        _float frameNorm = (keyFrame - m_iFrameMin + 0.6f) / static_cast<_float>(m_iFrameMax - m_iFrameMin);

        _float x = rc.Min.x + frameNorm * (rc.Max.x - rc.Min.x);
        _float y = rc.Min.y;
        _float h = rc.Max.y - rc.Min.y;

        draw_list->AddLine(ImVec2(x, y), ImVec2(x, y + h), IM_COL32(255, 255, 0, 255));
        draw_list->AddCircleFilled(ImVec2(x, y + h / 2), 3.0f, IM_COL32(255, 255, 0, 255));
    }
}
