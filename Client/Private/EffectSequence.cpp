#include "EffectSequence.h"
#include "EffectBase.h"

CEffectSequence::~CEffectSequence()
{
	for (auto& Item : m_Items)
	{
		Safe_Release(Item.pEffect);
	}
}

void CEffectSequence::BeginEdit(_int iIndex)
{

}

void CEffectSequence::EndEdit()
{

}

_int CEffectSequence::GetItemTypeCount() const
{
	return _int();
}

void CEffectSequence::Get(_int index, _int** start, _int** end, _int* type, _uint* color)
{
	if (color)  *color = m_Items[index].iColor;
	if (start) *start = m_Items[index].iStart;
	if (end)   *end = m_Items[index].iEnd;
	if (type)  *type = m_Items[index].iType;
}

void CEffectSequence::Add(_int type)
{
	//deprecated
	//m_Items.push_back({ "Name", 0, 10, 0, D3DCOLOR_ARGB(255, 255, 255, 0) });
}

void CEffectSequence::Add(string name, _int start, _int end, _int type, _uint color, CEffectBase* effect)
{
	//deprecated
	//m_Items.push_back(SEQDESC{ name, &start, &end, type, color, effect });
}

void CEffectSequence::Add(string name, _int start, _int end, _int type, _uint color)
{
	//deprecated
	//m_Items.push_back(SEQDESC{ name, start, end, type, color });
}

void CEffectSequence::Add(string name, CEffectBase* effect, _int type, _uint color)
{
	m_Items.push_back(SEQDESC{ name, effect->Get_StartTrackPosition_Ptr(), effect->Get_Duration_Ptr(), type, color, effect});
}


void CEffectSequence::Del(_int)
{

}

void CEffectSequence::Duplicate(_int)
{

}

void CEffectSequence::CustomDraw(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect&, const ImRect&, const ImRect&)
{
	auto& effect = m_Items[index].pEffect;
	for (auto& key : effect->Get_KeyFrames())
	{
		_float frameNorm = static_cast<_float>(key.fTrackPosition - GetFrameMin()) / (GetFrameMax() - GetFrameMin()) + 
			static_cast<_float>(*effect->Get_StartTrackPosition_Ptr() - GetFrameMin()) / (GetFrameMax() - GetFrameMin()) ;
		_float x = rc.Min.x + frameNorm * (rc.Max.x - rc.Min.x);
		_float y = rc.Min.y;
		_float h = rc.Max.y - rc.Min.y;

		//draw_list->AddLine(ImVec2(x, y), ImVec2(x, y + h), IM_COL32(255, 255, 0, 255));
		draw_list->AddCircleFilled(ImVec2(x, y + h / 2), 3.0f, IM_COL32(255, 255, 0, 255));
	}
}

void CEffectSequence::Copy()
{

}

void CEffectSequence::Paste()
{

}

void CEffectSequence::DoubleClick(_int)
{

}
