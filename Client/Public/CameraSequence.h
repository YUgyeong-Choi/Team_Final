#pragma once

#include "Base.h"

#include "Client_Defines.h"


NS_BEGIN(Client)

class CCameraSequence : public ImSequencer::SequenceInterface
{
public:
    struct CAMERA_KEY
    {
        _int startFrame, endFrame;
        _int type;
        _uint color;

        XMFLOAT3 position;
        XMFLOAT3 rotation;  
        XMFLOAT3 offSetRotation;
        _float fFov;

        INTERPOLATION_CAMERA interpPosition;
        INTERPOLATION_CAMERA interpRotation;
        INTERPOLATION_CAMERA interpFov;

        //_bool bLookAt = false;
        //_vector lookAtTarget;
        //INTERPOLATION_CAMERA interpLookAt;
        _int keyFrame;
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

    void InitAllFrames();

    // ½ÃÄö½º Ãß°¡
    void Add(_int startFrame, _int endFrame, _int type);
    void Add_KeyFrame(_int type, _int keyFrame);
    void Delete_KeyFrame(_int type, _int keyFrame);

    void Set_EndFrame(_int endFrame);

    virtual size_t GetCustomHeight(int /*index*/) { return 12; }
    virtual void CustomDraw(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect&, const ImRect&, const ImRect&) override;

    CAMERA_KEY* GetKeyAtFrame(_int frame)
    {
        for (auto& key : m_vecKeys)
        {
            if (frame >= key.startFrame && frame <= key.endFrame)
                return &key;
        }
        return nullptr;
    }
public:
    vector<CAMERA_KEY> m_vecKeys;
    vector<_int> m_vecPosKeyFrames;
    vector<_int> m_vecRotKeyFrames;
    vector<_int> m_vecFovKeyFrames;
    _int m_iFrameMax = { 80 };
    _int m_iFrameMin = { 0 };
};

NS_END