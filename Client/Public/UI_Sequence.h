#pragma once
#include "Base.h"	
#define USE_IMGUI
#include "Client_Defines.h"
#undef USE_IMGUI

NS_BEGIN(Engine)
class CAnimation;
NS_END

NS_BEGIN(Client)
class CUI_Sequence : public ImSequencer::SequenceInterface
{
public:
    struct SequenceItem
    {
        //뭘 넣지

        int start, end, type;
        unsigned int color;
        string name; // 이름
    };
public:
    CUI_Sequence() = delete;
    CUI_Sequence(vector<SequenceItem>& items, class CToolbar* pToolbar) : m_items(items) {}
    virtual ~CUI_Sequence() = default;

    _int GetFrameMin()   const override { return m_frameMin; }
    _int GetFrameMax()   const override { return m_frameMax; }
    // 이벤트(아이템) 개수
    _int GetItemCount()  const override { return (_int)m_items.size(); }
    // 각 아이템의 시작·끝 프레임, 타입, 색상 리턴
    void Get(int index, int** start, int** end, int* type, unsigned int* color) override
    {
        if (index < 0 || index >= static_cast<_int>(m_items.size()))
            return;
        if (start != nullptr)
            *start = &m_items[index].start;
        if (end != nullptr)
            *end = &m_items[index].end;
        if (type != nullptr)
            *type = m_items[index].type;
        if (color != nullptr)
            *color = m_items[index].color;
    }
    const _char* GetItemLabel(_int index) const override
    {
        return m_items[index].name.c_str();
    }

    void Add(_int type)     override;
    void Del(_int index)    override;

    _int  m_frameMin = 0;
    _int  m_frameMax = 60;
    _int  m_iCurEditIndex = -1;

private:
    vector<SequenceItem>& m_items;

};
NS_END

