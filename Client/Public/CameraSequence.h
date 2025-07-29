#pragma once

#include "Base.h"

#include "Client_Defines.h"


NS_BEGIN(Client)

class CCameraSequence : public ImSequencer::SequenceInterface
{
public:
    struct CAMERA_KEY
    {
        int startFrame;
        int endFrame;
        int type;
        unsigned int color;
        _matrix worldMatrix = XMMatrixIdentity();
        INTERPOLATION_CAMERA interpWorld = INTERPOLATION_CAMERA::NONE;
        _float fFov = 60.f;
        INTERPOLATION_CAMERA interpFov = INTERPOLATION_CAMERA::NONE;
        _vector rotation = XMVectorZero();
        INTERPOLATION_CAMERA interpRotation = INTERPOLATION_CAMERA::NONE;
    };
public:
    CCameraSequence() = default;
    virtual ~CCameraSequence();

public:
    virtual int GetFrameMin() const override { return m_iFrameMin; }
    virtual int GetFrameMax() const override { return m_iFrameMax; }
    virtual int GetItemCount() const override { return static_cast<int>(m_vecKeys.size()); }
    virtual int GetItemTypeCount() const override { return 3; }

    virtual const char* GetItemTypeName(int typeIndex) const override
    {
        switch (typeIndex)
        {
        case 0: return "Position";
        case 1: return "Rotation";
        case 2: return "Fov";
        default: return "Unknown";
        }
    }

    virtual void Get(int index, int** start, int** end, int* type, unsigned int* color) override
    {
        if(start) *start = &m_vecKeys[index].startFrame;
        if (end) *end = &m_vecKeys[index].endFrame;
        if (type) *type = m_vecKeys[index].type;
        if (color) *color = m_vecKeys[index].color;
    }

    // ½ÃÄö½º ÀÌ¸§
    virtual const char* GetItemLabel(int index) const override
    {
        static char label[64];
        switch (index)
        {
        case 0: return "Position";
            sprintf_s(label, "Camera Key %d", index);
        case 1: return "Rotation";
            sprintf_s(label, "Camera Key %d", index);
        case 2: return "Fov";
            sprintf_s(label, "Camera Key %d", index);
        default: return "Unknown";
        }

        return label;
    }

    // ½ÃÄö½º Ãß°¡
    void Add(_int startFrame, _int endFrame, int type);
    
    void Set_EndFrame(_int endFrame);

public:
    std::vector<CAMERA_KEY> m_vecKeys;
    _int m_iFrameMax = { 80 };
    _int m_iFrameMin = { 0 };
};

NS_END