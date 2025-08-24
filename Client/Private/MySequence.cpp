#include "AnimController.h"
#include "MySequence.h"
#include "Animation.h"
#include "Animator.h"


CMySequence::~CMySequence()
{
    Safe_Release(m_pAnimator); 
}

void CMySequence::Get(int index, int** start, int** end, int* type, unsigned int* color)
{
    CAnimation* pCurAnim = m_pAnimator->GetCurrentAnim();

	if (pCurAnim == nullptr)
	{
		return;
	}

    m_iStart = 0;
    // 60 프레임 기준으로 프레임 계산
    m_iEnd = static_cast<_int>(pCurAnim->GetDuration());
    m_frameMax = m_iEnd;
	m_iType = 1; // 기본 타입

     if (start != nullptr)
        *start = &m_iStart;
    if (end != nullptr)
        *end = &m_iEnd;
    if (type != nullptr)
        *type = m_iType;
    if (color != nullptr)
		*color = m_uiColor;
}

_int CMySequence::GetItemCount() const
{
    return m_pAnimator->GetCurrentAnim() ? 1 : 0;
}

const _char* CMySequence::GetItemLabel(_int index) const
{
    auto p = m_pAnimator->GetCurrentAnim();
    return p ? p->Get_Name().c_str() : "";
}

void CMySequence::SetAnimator(CAnimator* pAnimator)
{
    m_pAnimator = pAnimator;
}

