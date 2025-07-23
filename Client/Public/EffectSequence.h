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
    virtual int GetFrameMin() const override { return 0; };
    virtual int GetFrameMax() const override { return 100; };
    virtual int GetItemCount() const override { return m_Items.size(); };

    virtual void BeginEdit(int /*index*/)  override {}
    virtual void EndEdit() override {}
    virtual int GetItemTypeCount() const  override { return 0; }
    virtual const char* GetItemTypeName(int /*typeIndex*/) const override { return ""; }
    virtual const char* GetItemLabel(int index) const override { return m_Items[index].name.c_str(); }
    virtual const char* GetCollapseFmt() const override { return "%d Frames / %d entries"; }

    virtual void Get(int index, int** start, int** end, int* type, unsigned int* color) override {
        if (color)  *color = m_Items[index].color;
        if (start) *start = &m_Items[index].startFrame;
        if (end)   *end = &m_Items[index].endFrame;
        if (type)  *type = m_Items[index].type;
    };
    virtual void Add(int type)  override { m_Items.push_back({ "Name", 0, 10, 1, 100 }); }
    virtual void Del(int /*index*/) override {}
    virtual void Duplicate(int /*index*/) override {}

    virtual void Copy() override {}
    virtual void Paste()  override {}

    virtual size_t GetCustomHeight(int /*index*/) override { return 0; }
    virtual void DoubleClick(int /*index*/) override {}

private:
    struct SequenceItem
    {
        string name;
        _int startFrame = 0;
        _int endFrame = 0;
        _int type = 0; // 타입에 따라 색상/분류 등 처리 가능
        _uint color = 255;
    };

    std::vector<SequenceItem> m_Items;
private:
    //vector<class CGameObject*> Effects;
};

NS_END