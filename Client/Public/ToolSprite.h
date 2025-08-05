#pragma once

#include "Client_Defines.h"
#include "SpriteEffect.h"

NS_BEGIN(Client)

class CToolSprite final : public CSpriteEffect
{
public:
	typedef struct tagToolSEDesc : public CSpriteEffect::DESC
	{
		_bool bLoadingInTool = { false };
	}DESC;
private:
	CToolSprite(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CToolSprite(const CToolSprite& Prototype);
	virtual ~CToolSprite() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

private:
	virtual HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

private:
	_bool m_bLoadingInTool = { false };
	
public:
	static CToolSprite* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END