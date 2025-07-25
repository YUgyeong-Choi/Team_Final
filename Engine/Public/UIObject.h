#pragma once

#include "GameObject.h"

/* CUIObject_2D */
/* CUIObject_3D */

NS_BEGIN(Engine)

class ENGINE_DLL CUIObject abstract : public CGameObject
{
public:
	typedef struct tagUIObjectDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_float			fX, fY, fSizeX, fSizeY;
		_float			fOffset = {0.f};
	}UIOBJECT_DESC;

	_float Get_Depth() { return m_fOffset; }

	_bool  Get_isFade() { return m_isFade; }
	_float Get_Alpha() { return m_fCurrentAlpha; }

protected:
	CUIObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUIObject(const CUIObject& Prototype);
	virtual ~CUIObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	// ���߿� ����, start�� ũ���ϸ� fade out, �۰��ϸ� fade in
	void FadeStart(_float fStartAlpha, _float fEndAlpha, _float fDuration);
	

	void Fade(_float fTimeDelta);
	
	//

protected:
	/* ����Ʈ ���� �������� �߽���ġ fX, fY, ������ fSiuzeX, fSizeY */
	_float			m_fX{}, m_fY{}, m_fSizeX{}, m_fSizeY{};
	_float			m_fOffset = {0.f};

	/* ���� ������ ���� ���. */
	_float4x4		m_ViewMatrix{}, m_ProjMatrix{};

	// ���߿� ����, fade ��
	_bool			m_isFade = {false};
	_bool			m_isFadeIn = {};
	_float			m_fFadeTime = {};
	_float			m_fFadeElapsedTime = {};
	_float			m_fCurrentAlpha = {};
	_float			m_fStartAlpha = {};
	_float			m_fEndAlpha = {};
	//
	

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

NS_END