#pragma once
#include "GameObject.h"

#include "Client_Defines.h"

NS_BEGIN(Engine)
class CNavigation;
NS_END

NS_BEGIN(Client)

class CNav final : public CGameObject
{
public:
	typedef struct tagNavDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_uint iLevelIndex = { ENUM_CLASS(LEVEL::END) };
	} NAV_DESC;

private:
	CNav(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CNav(const CNav& Prototype);
	virtual ~CNav() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta)override;
	virtual void Update(_float fTimeDelta)override;
	virtual void Late_Update(_float fTimeDelta)override;
	virtual HRESULT Render()override;

private:
	_bool m_bVisible = { false }; //·»´õ¸µ ¿©ºÎ

private:
	CNavigation* m_pNavigationCom = { nullptr };

private:
	HRESULT Ready_Components(void* pArg);

public:
	static CNav* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END