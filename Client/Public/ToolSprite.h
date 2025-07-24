#pragma once

#include "Client_Defines.h"
//#include "BlendObject.h"
#include "SpriteEffect.h"

NS_BEGIN(Client)

class CToolSpirte final : public CSpriteEffect
{
private:
	CToolSpirte(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CToolSpirte(const CToolSpirte& Prototype);
	virtual ~CToolSpirte() = default;

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


public:
	static CToolSpirte* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END