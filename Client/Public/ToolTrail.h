#pragma once

#include "Client_Defines.h"
#include "TrailEffect.h"

NS_BEGIN(Client)

class CToolTrail final : public CTrailEffect
{
public:
	typedef struct tagToolSEDesc : public CTrailEffect::DESC
	{
		_bool bLoadingInTool = { false };
	}DESC;
private:
	CToolTrail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CToolTrail(const CToolTrail& Prototype);
	virtual ~CToolTrail() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	virtual void Update_Tool(_float fTimeDelta, _float fCurFrame);

private:
	virtual HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

private:
	_bool m_bLoadingInTool = { false };
	
public:
	static CToolTrail* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END