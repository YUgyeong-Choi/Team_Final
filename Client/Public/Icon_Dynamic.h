#pragma once

#include "Dynamic_UI.h"

NS_BEGIN(Client)

class CIcon_Dynamic final : public CDynamic_UI
{
private:
	CIcon_Dynamic(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CIcon_Dynamic(const CIcon_Dynamic& Prototype);
	virtual ~CIcon_Dynamic() = default;

public:
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

private:
	HRESULT Ready_Components(const wstring& strTextureTag);

private:
	// 나중에 플레이어랑 연동
	_int	m_iAmount = { 100 };

public:
	static CIcon_Dynamic* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;


};

NS_END