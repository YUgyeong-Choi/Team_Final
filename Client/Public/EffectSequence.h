#pragma once

#include "Base.h"

#define USE_IMGUI
#include "Client_Defines.h"
#undef USE_IMGUI


NS_BEGIN(Client)

class CEffectSequence : public ImSequencer::SequenceInterface
{
public:
    CEffectSequence() = default;
    ~CEffectSequence() = default;

public:
    virtual _int GetFrameMin() const override { return 0; };
    virtual _int GetFrameMax() const override { return 150; };
    virtual _int GetItemCount() const override { return static_cast<_int>(m_Items.size()); };

    virtual void BeginEdit(_int iIndex)  override;
    virtual void EndEdit() override;
    virtual _int GetItemTypeCount() const  override;
    virtual const _char* GetItemTypeName(_int iTypeIndex) const override { return ""; }
    virtual const _char* GetItemLabel(_int index) const override { return m_Items[index].strName.c_str(); }

    virtual void Get(_int index, _int** start, _int** end, _int* type, _uint* color) override;
    virtual void Add(_int type)  override;
    virtual void Add(string name, _int start, _int end, _int type, _uint color);
    virtual void Add(string name, _int start, _int end, _int type, _uint color, class CEffectBase* effect);
    virtual void Del(_int index) override;
    virtual void Duplicate(_int index) override;

    virtual void CustomDraw(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect&, const ImRect&, const ImRect&) override;

    virtual void Copy() override;
    virtual void Paste()  override;

    virtual void DoubleClick(_int /*index*/) override;

public:
    typedef struct SequenceItem
    {
        string strName;
        _int iStart = 0;
        _int iEnd = 0;
        _int iType = 0; // 타입에 따라 색상/분류 등 처리 가능
        _uint iColor = D3DCOLOR_ARGB(255, 255, 255, 255);
        class CEffectBase* pEffect = { nullptr };
    }SEQDESC;

    vector<SequenceItem>        m_Items;
};

NS_END