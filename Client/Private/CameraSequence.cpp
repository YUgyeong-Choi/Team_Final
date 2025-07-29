#include "CameraSequence.h"

CCameraSequence::~CCameraSequence()
{

}

void CCameraSequence::Add(_int startFrame, _int endFrame, int type)
{
    CAMERA_KEY newKey;
    newKey.startFrame = startFrame;
    newKey.endFrame = endFrame;
    newKey.type = type;
    newKey.worldMatrix = XMMatrixIdentity();
    newKey.interpWorld = INTERPOLATION_CAMERA::NONE;
    newKey.fFov = 60.f;
    newKey.interpFov = INTERPOLATION_CAMERA::NONE;
    newKey.rotation = XMVectorZero();
    newKey.interpRotation = INTERPOLATION_CAMERA::NONE;

    // Ÿ�Կ� ���� ���� �б�
    switch (type)
    {
    case 0: newKey.color = IM_COL32(255, 200, 0, 255); break;      // Position: Yellow
    case 1: newKey.color = IM_COL32(100, 255, 255, 255); break;    // Rotation: Cyan
    case 2: newKey.color = IM_COL32(200, 100, 255, 255); break;    // FOV: Purple
    }

    // �ʱⰪ�� �ʿ�� �б� ���� ����
    m_vecKeys.push_back(newKey);
}

void CCameraSequence::Set_EndFrame(_int endFrame)
{
    for (auto& key : m_vecKeys)
        key.endFrame = endFrame;
}
