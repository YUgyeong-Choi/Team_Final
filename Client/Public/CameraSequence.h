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
        case 2: return "FOV";
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

    virtual const char* GetItemLabel(int index) const override
    {
        static char label[64];
        sprintf_s(label, "Camera Key %d", index);
        return label;
    }

    void Add(int type) override
    {
        CAMERA_KEY newKey;
        newKey.startFrame = 10;
        newKey.endFrame = 20;
        newKey.type = type;

        // 타입에 따른 색상 분기
        switch (type)
        {
        case 0: newKey.color = IM_COL32(255, 200, 0, 255); break;      // Position: Yellow
        case 1: newKey.color = IM_COL32(100, 255, 255, 255); break;    // Rotation: Cyan
        case 2: newKey.color = IM_COL32(200, 100, 255, 255); break;    // FOV: Purple
        }

        // 초기값도 필요시 분기 설정 가능
        m_vecKeys.push_back(newKey);
    }

    virtual void Del(int index) override
    {
        if (index >= 0 && index < (int)m_vecKeys.size())
            m_vecKeys.erase(m_vecKeys.begin() + index);
    }

    virtual void Duplicate(int index) override
    {
        if (index >= 0 && index < (int)m_vecKeys.size())
        {
            CAMERA_KEY copy = m_vecKeys[index];
            copy.startFrame += 5;
            copy.endFrame += 5;
            m_vecKeys.push_back(copy);
        }
    }

public:
    std::vector<CAMERA_KEY> m_vecKeys;
    _int m_iFrameMax = { 80 };
    _int m_iFrameMin = { 0 };
};

NS_END