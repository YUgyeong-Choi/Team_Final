#pragma once

#include "GameObject.h"

/* ������ �ʿ��� ��ü���� ī�޶�� ������ ���̸� ����Ͽ��� �����Ѵ�. */
/* ���̸� ���� ����. */

NS_BEGIN(Engine)

class ENGINE_DLL CBlendObject abstract : public CGameObject
{
protected:
	CBlendObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBlendObject(const CBlendObject& Prototype);
	virtual ~CBlendObject() = default;

public:
	_float Get_Depth() const {
		return m_fDepth;
	}

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

protected:
	/* ī�޶�κ����� ���̸� ����Ѵ�.  */
	_float				m_fDepth = {};

protected:
	void Compute_CameraDepth(_fvector vPosition);
	
public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free();
};

NS_END