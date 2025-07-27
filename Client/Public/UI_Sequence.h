#pragma once
#include "Base.h"	
#define USE_IMGUI
#include "Client_Defines.h"
#undef USE_IMGUI

NS_BEGIN(Engine)

NS_END

NS_BEGIN(Client)
class CUI_Sequence : public ImSequencer::SequenceInterface
{
  
public:
    CUI_Sequence() = default;
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
            *start = &m_items[index].iStartFrame;
        if (end != nullptr)
            *end = &m_items[index].iEndFrame;
        if (type != nullptr)
            *type = m_items[index].iType;
        if (color != nullptr)
            *color = m_items[index].color;
    }
    const _char* GetItemLabel(_int index) const override
    {
        return m_items[index].strTypeTag.c_str();
    }

    void Add(_int type)     override;
    void Del(_int index)    override;

    // 어떤 인덱스 아이템을 가져와서 수정할지 결정
    void Upadte_Items(_int iIndex, UI_FEATURE_TOOL_DESC& eDesc);

    void Clear() { m_items.clear(); }

    UI_FEATURE_TOOL_DESC Get_Desc(_int iIndex) { return m_items[iIndex]; }

    void Push_Item(UI_FEATURE_TOOL_DESC& eDesc);

    _int  m_frameMin = 0;
    _int  m_frameMax = 120;
    _int  m_iCurEditIndex = -1;

private:
    vector<UI_FEATURE_TOOL_DESC> m_items;

};
NS_END

