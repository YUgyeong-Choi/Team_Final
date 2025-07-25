#include "UI_Sequence.h"

void CUI_Sequence::Add(_int type)
{
	SequenceItem eDesc;
	eDesc.start = 0;
	eDesc.end = 60;
	eDesc.type = type;



	if (type == 0)
		eDesc.name = "Fade";
	else if (type == 1)
		eDesc.name = "Move";
	else if (type == 2)
		eDesc.name = "UV";

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
