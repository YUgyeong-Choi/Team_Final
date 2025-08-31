#include "UI_Sequence.h"

void CUI_Sequence::Add(_int type)
{
	UI_FEATURE_TOOL_DESC eDesc;

	eDesc.iStartFrame = 0;
	eDesc.iEndFrame = 60;
	eDesc.iType = type;

	if (type == 0)
		eDesc.strTypeTag = "Fade";
	else if (type == 1)
		eDesc.strTypeTag = "UV";
	else if (type == 2)
		eDesc.strTypeTag = "Pos";
	else if (type == 3)
		eDesc.strTypeTag = "Scale";
	else if (type == 4)
		eDesc.strTypeTag = "Rotation";

	// Èò»ö?
	eDesc.color = 0xFFFFFFFF;

	m_items.push_back(eDesc);

}

void CUI_Sequence::Del(_int index)
{
	if (index < 0 || index >= m_items.size())
		return;

	m_items.erase(m_items.begin() + index);
}

void CUI_Sequence::Upadte_Items(_int iIndex, UI_FEATURE_TOOL_DESC& eDesc)
{


	m_items[iIndex] = eDesc;

	if (eDesc.iType == 0)
		eDesc.strTypeTag = "Fade";
	else if (eDesc.iType == 1)
		eDesc.strTypeTag = "UV";
	else if (eDesc.iType == 2)
		eDesc.strTypeTag = "Pos";
	else if (eDesc.iType == 3)
		eDesc.strTypeTag = "Scale";
	else if (eDesc.iType == 4)
		eDesc.strTypeTag = "Rotation";
}

void CUI_Sequence::Push_Item(UI_FEATURE_TOOL_DESC& eDesc)
{
	m_items.push_back(eDesc);
}
