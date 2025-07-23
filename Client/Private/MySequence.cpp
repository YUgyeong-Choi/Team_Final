#include "MySequence.h"
#include "Animation.h"


void CMySequence::Add(_int type)
{
    SequenceItem item;
    item.pAnim = nullptr;
    item.start = 0;
    item.end = 60;
    item.type = type;
    item.color = 0xFF00CCFF;
    item.name = "NewEffect";
    m_items.push_back(item);
}

void CMySequence::Del(_int index)
{
	if (index < 0 || index >= (_int)m_items.size())
		return;
    Safe_Release(m_items[index].pAnim);
	m_items.erase(m_items.begin() + index);
}
