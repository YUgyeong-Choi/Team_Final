#pragma once
#include "Base.h"	
#define USE_IMGUI
#include "Client_Defines.h"
#undef USE_IMGUI

NS_BEGIN(Engine)
class CAnimation;
class CAnimator;
NS_END

NS_BEGIN(Client)
class CMySequence : public ImSequencer::SequenceInterface
{
public:
    CMySequence() = default;
    virtual ~CMySequence();

    _int GetFrameMin()   const override { return m_frameMin; }
    _int GetFrameMax()   const override { return m_frameMax; }

    // 각 아이템의 시작·끝 프레임, 타입, 색상 리턴
    void Get(int index, int** start, int** end, int* type, unsigned int* color) override;

    _int GetItemCount() const override;

    const _char* GetItemLabel(_int index) const override;


    // 일단 동작 막기
    void Add(_int type)     override {};
    void Del(_int index)    override {};

    void SetAnimator(CAnimator* pAnimator);

private:
    _int  m_frameMin = 0;
    _int  m_frameMax = 60;
    _int  m_iCurEditIndex = -1;

    _int m_iStart{ 0 };
	_int m_iEnd{ 0 };
	_int m_iType{ 0 };
	_uint m_uiColor{ 0xFF00CCFF }; // 기본 색상

private:
    CAnimator* m_pAnimator{ nullptr };

};
NS_END

